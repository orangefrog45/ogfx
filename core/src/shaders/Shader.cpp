#include "ogfx/pch.h"
#include "ogfx/shaders/Shader.h"

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

Shader& Shader::AddStage(vk::ShaderStageFlagBits shader_stage_flags, const std::string& spv_path) {
    for (auto stage: m_shader_stages) {
        if (shader_stage_flags == stage) OGFX_ASSERT_STR(false, "Cannot add duplicate shader stage");
    }

    std::vector<std::byte>& spv = m_spv_storage.emplace_back();
    ReadBinaryFile(spv_path, spv);

    OGFX_ASSERT(spv.size() % 4 == 0);

    if (!m_pipeline_layout) ReflectPipelineLayout(spv);

    vk::ShaderCreateInfoEXT& shader_create_info = m_shader_create_infos.emplace_back();
    shader_create_info.stage = shader_stage_flags;
    shader_create_info.codeType = vk::ShaderCodeTypeEXT::eSpirv;
    shader_create_info.codeSize = spv.size();
    shader_create_info.pCode = spv.data();
    shader_create_info.pName = "main";
    shader_create_info.setLayoutCount = m_set_layouts.size();
    shader_create_info.pSetLayouts = m_set_layouts.data();
    shader_create_info.pushConstantRangeCount = m_push_constant_ranges.size();
    shader_create_info.pPushConstantRanges = m_push_constant_ranges.data();

    m_shader_stages.emplace_back(shader_stage_flags);
    return *this;
}

vk::PipelineLayout Shader::GetPipelineLayout() {
    return *m_pipeline_layout;
}

void Shader::Bind(vk::CommandBuffer& cmd) {
    cmd.bindShadersEXT(m_shaders.size(), m_shader_stages.data(), m_shaders.data());
}

void Shader::Build() {
    m_shaders.resize(m_shader_create_infos.size());
    OGFX_VK_CHECK(VkContext::GetLogicalDevice().device.createShadersEXT(
        m_shader_create_infos.size(), m_shader_create_infos.data(), nullptr, m_shaders.data()));

    m_spv_storage.clear();
    m_shader_create_infos.clear();
}

void Shader::ReflectPipelineLayout(const std::vector<std::byte>& spv) {
    if (m_pipeline_layout) return;

    spirv_cross::Compiler compiler{reinterpret_cast<const uint32_t*>(spv.data()), spv.size() / 4};
    auto resources = compiler.get_shader_resources();

    struct DescriptorSetLayout {
        uint32_t set;
        vk::DescriptorSetLayoutCreateFlags flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };

    std::unordered_map<uint32_t, DescriptorSetLayout> sets;

    for (const spirv_cross::Resource& resource : resources.storage_images) {
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

        vk::DescriptorSetLayoutBinding& binding = sets[set].bindings.emplace_back();
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.descriptorType = vk::DescriptorType::eStorageImage;
        binding.descriptorCount = 1;
        binding.stageFlags = vk::ShaderStageFlagBits::eAll; // TODO: Make this more specific
    }

    for (const spirv_cross::Resource& resource : resources.push_constant_buffers) {
        vk::PushConstantRange& push_constant_range = m_push_constant_ranges.emplace_back();
        push_constant_range.offset = compiler.get_decoration(resource.id, spv::DecorationOffset);
        push_constant_range.size = compiler.get_declared_struct_size(compiler.get_type(resource.type_id));
        push_constant_range.stageFlags = vk::ShaderStageFlagBits::eAll; // TODO: Make this more specific
    }

    m_set_layouts.resize(sets.size());

    int idx = 0;
    for (const auto& set: sets | std::views::values) {
        vk::DescriptorSetLayoutCreateInfo info{};
        info.bindingCount = set.bindings.size();
        info.pBindings = set.bindings.data();
        info.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptor;

        OGFX_VK_CHECK(ogfx::VkContext::GetLogicalDevice().device.createDescriptorSetLayout(
            &info, nullptr, &m_set_layouts[idx++]));
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.setLayoutCount = m_set_layouts.size();
    pipeline_layout_create_info.pSetLayouts = m_set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = m_push_constant_ranges.size();
    pipeline_layout_create_info.pPushConstantRanges = m_push_constant_ranges.data();

    m_pipeline_layout = ogfx::VkContext::GetLogicalDevice().device.createPipelineLayoutUnique(pipeline_layout_create_info);
}
