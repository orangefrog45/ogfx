#pragma once

#include <span>
#include <ranges>
#include <unordered_map>

// NEVER change this, it needs to use this set of headers and not the built-in vulkan ones
#include <../extern/spirv-cross/spirv_cross.hpp>

#include "ogfx/VkContext.h"

namespace ogfx {
    class Shader {
    public:
        ~Shader();

        Shader& AddStage(vk::ShaderStageFlagBits shader_stage_flags,
                         const std::string& spv_path);
        Shader& AddStage(vk::ShaderStageFlagBits shader_stage_flags,
                         std::span<const char> spv_data);
        void Bind(vk::CommandBuffer& cmd);
        void Build();
        [[nodiscard]] vk::PipelineLayout GetPipelineLayout();

    private:
        void ReflectPipelineLayout(std::span<char> spv);

        std::vector<vk::DescriptorSetLayout> m_set_layouts;
        std::vector<vk::PushConstantRange> m_push_constant_ranges;

        std::vector<vk::ShaderEXT> m_shaders;
        std::vector<vk::ShaderCreateInfoEXT> m_shader_create_infos;
        std::vector<vk::ShaderStageFlagBits> m_shader_stages;

        std::vector<std::vector<char>> m_spv_storage;

        vk::UniquePipelineLayout m_pipeline_layout;
    };
}
