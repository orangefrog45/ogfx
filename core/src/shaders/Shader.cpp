#include "ogfx/shaders/Shader.h"
#include "ogfx/pch.h"

#include "otl/Files.h"

using namespace ogfx;

Shader::~Shader() {
    auto& device = VkContext::GetLogicalDevice().device;

    for (auto shader : m_shaders) {
        device.destroyShaderEXT(shader);
    }

    for (auto layout : m_set_layouts) {
        device.destroyDescriptorSetLayout(layout);
    }
}

Shader& Shader::AddStage(vk::ShaderStageFlagBits shader_stage_flags,
                         const std::string& spv_path, std::string entry_point) {
    std::vector<std::byte> spv;
    if (!otl::ReadBinaryFile(spv_path, spv)) {
        log::Error("Failed to read shader file: '{}'", spv_path);
        return *this;
    }

    return AddStage(shader_stage_flags, std::span{
                        reinterpret_cast<char*>(spv.data()), spv.size()
                    }, std::move(entry_point));
}

Shader& Shader::AddStage(vk::ShaderStageFlagBits shader_stage_flags,
                         std::span<const char> spv_data, std::string entry_point) {
    for (auto stage : m_shader_stages) {
        if (shader_stage_flags == stage)
            OGFX_ASSERT(false, "Cannot add duplicate shader stage");
    }

    std::vector<char>& spv = m_spv_storage.emplace_back(
        spv_data.begin(), spv_data.end());

    OGFX_ASSERT(spv.size() % 4 == 0);

    if (!m_pipeline_layout)
        ReflectPipelineLayout(spv);

    vk::ShaderCreateInfoEXT& shader_create_info =
        m_shader_create_infos.emplace_back();
    shader_create_info.stage = shader_stage_flags;
    shader_create_info.codeType = vk::ShaderCodeTypeEXT::eSpirv;
    shader_create_info.codeSize = spv.size();
    shader_create_info.pCode = spv.data();
    // Shader objects are created unlinked, so a vertex shader must declare what may follow it
    if (shader_stage_flags == vk::ShaderStageFlagBits::eVertex)
        shader_create_info.nextStage = vk::ShaderStageFlagBits::eFragment;
    shader_create_info.setLayoutCount = (uint32_t)m_set_layouts.size();
    shader_create_info.pSetLayouts = m_set_layouts.data();
    shader_create_info.pushConstantRangeCount =
        (uint32_t)m_push_constant_ranges.size();
    shader_create_info.pPushConstantRanges = m_push_constant_ranges.data();

    m_shader_stages.emplace_back(shader_stage_flags);
    m_entry_points.emplace_back(std::move(entry_point));
    return *this;
}

vk::PipelineLayout Shader::GetPipelineLayout() { return *m_pipeline_layout; }

bool Shader::HasBinding(uint32_t set, uint32_t binding) const {
    return std::ranges::contains(m_bindings, std::pair{set, binding});
}

void Shader::PushDescriptorSet(vk::CommandBuffer cmd, vk::PipelineBindPoint bind_point,
                               uint32_t set,
                               std::span<const vk::WriteDescriptorSet> writes) {
    std::vector<vk::WriteDescriptorSet> used_writes;
    used_writes.reserve(writes.size());
    for (const auto& write : writes) {
        if (HasBinding(set, write.dstBinding))
            used_writes.push_back(write);
    }

    if (used_writes.empty())
        return;

    cmd.pushDescriptorSet(bind_point, *m_pipeline_layout, set,
                          (uint32_t)used_writes.size(), used_writes.data());
}

void Shader::Bind(vk::CommandBuffer& cmd) {
    cmd.bindShadersEXT((uint32_t)m_shaders.size(),
                       m_shader_stages.data(),
                       m_shaders.data());
}

void Shader::Build() {
    // Set here as m_entry_points may have reallocated since each stage was added
    for (size_t i = 0; i < m_shader_create_infos.size(); i++)
        m_shader_create_infos[i].pName = m_entry_points[i].c_str();

    m_shaders.resize(m_shader_create_infos.size());
    OGFX_VK_CHECK(VkContext::GetLogicalDevice().device.createShadersEXT(
        (uint32_t)m_shader_create_infos.size(), m_shader_create_infos.data(),
        nullptr, m_shaders.data()));

    m_spv_storage.clear();
    m_shader_create_infos.clear();
    m_entry_points.clear();
}

void Shader::ReflectPipelineLayout(std::span<char> spv) {
    if (m_pipeline_layout)
        return;

    spirv_cross::Compiler compiler{
        reinterpret_cast<const uint32_t*>(spv.data()),
        spv.size() / 4
    };
    struct DescriptorSetLayout {
        uint32_t set;
        vk::DescriptorSetLayoutCreateFlags flags =
            vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };

    std::unordered_map<uint32_t, DescriptorSetLayout> sets;

    auto add_bindings = [&](const auto& resource_list, vk::DescriptorType type,
                            vk::ShaderStageFlags stages = vk::ShaderStageFlagBits::eAll) {
        for (const spirv_cross::Resource& resource : resource_list) {
            uint32_t set =
                compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
            uint32_t binding_idx =
                compiler.get_decoration(resource.id, spv::DecorationBinding);

            // Entry points in the same module can share resources
            if (HasBinding(set, binding_idx))
                continue;

            vk::DescriptorSetLayoutBinding& binding = sets[set].bindings.emplace_back();
            binding.binding = binding_idx;
            m_bindings.emplace_back(set, binding.binding);
            binding.descriptorType = type;
            binding.descriptorCount = 1;
            binding.stageFlags = stages; // TODO: Make this more specific
        }
    };

    // Resources are only reported for the active entry point, so each one is reflected
    for (const spirv_cross::EntryPoint& entry_point : compiler.get_entry_points_and_stages()) {
        compiler.set_entry_point(entry_point.name, entry_point.execution_model);
        auto resources = compiler.get_shader_resources();

        add_bindings(resources.storage_images, vk::DescriptorType::eStorageImage);
        add_bindings(resources.uniform_buffers, vk::DescriptorType::eUniformBuffer);
        add_bindings(resources.storage_buffers, vk::DescriptorType::eStorageBuffer);
        add_bindings(resources.separate_images, vk::DescriptorType::eSampledImage);
        // Input attachments are only valid in fragment shaders
        add_bindings(resources.subpass_inputs, vk::DescriptorType::eInputAttachment,
                     vk::ShaderStageFlagBits::eFragment);

        for (const spirv_cross::Resource& resource :
             resources.push_constant_buffers) {
            uint32_t size = (uint32_t)compiler.get_declared_struct_size(
                compiler.get_type(resource.type_id));

            // Every range covers all stages, so one range large enough for each entry point
            if (!m_push_constant_ranges.empty()) {
                m_push_constant_ranges[0].size = std::max(m_push_constant_ranges[0].size, size);
                continue;
            }

            vk::PushConstantRange& push_constant_range =
                m_push_constant_ranges.emplace_back();
            push_constant_range.offset =
                compiler.get_decoration(resource.id, spv::DecorationOffset);
            push_constant_range.size = size;
            push_constant_range.stageFlags =
                vk::ShaderStageFlagBits::eAll; // TODO: Make this more specific
        }
    }

    m_set_layouts.resize(sets.size());

    uint32_t idx = 0;
    for (const auto& set : sets | std::views::values) {
        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = (uint32_t)set.bindings.size();
        info.pBindings = set.bindings.data();
        info.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor;

        OGFX_VK_CHECK(
            ogfx::VkContext::GetLogicalDevice().device.createDescriptorSetLayout(
                &info, nullptr, &m_set_layouts[idx++]));
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.setLayoutCount = (uint32_t)m_set_layouts.size();
    pipeline_layout_create_info.pSetLayouts = m_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount =
        (uint32_t)m_push_constant_ranges.size();
    pipeline_layout_create_info.pPushConstantRanges =
        m_push_constant_ranges.data();

    m_pipeline_layout =
        ogfx::VkContext::GetLogicalDevice().device.createPipelineLayoutUnique(
            pipeline_layout_create_info);
}
