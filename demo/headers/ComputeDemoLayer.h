#pragma once

#include "ogfx/Engine.h"
#include "ogfx/Gui.h"
#include "ogfx/VkCommands.h"
#include "ogfx/VkCommon.h"
#include "ogfx/layers/LayerStack.h"
#include "ogfx/VkContext.h"
#include "ogfx/Window.h"
#include "ogfx/shaders/Shader.h"
#include "ogfx/util/Util.h"

class ComputeDemoLayer : public ogfx::Layer {
public:
    ComputeDemoLayer(ogfx::Window& window) : m_window(window) {}
    ~ComputeDemoLayer() override = default;

    void Init() override {
        for (auto& cmd : m_cmd_bufs) cmd.Init(vk::CommandBufferLevel::ePrimary);
        for (auto& cmd : m_transition_cmd_bufs) cmd.Init(vk::CommandBufferLevel::ePrimary);

        vk::SemaphoreCreateInfo semaphore_create_info{};
        vk::FenceCreateInfo fence_create_info{};
        fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;

        for (int i = 0; i < ogfx::MAX_FRAMES_IN_FLIGHT; i++) {
            OGFX_VK_CHECK(ogfx::VkContext::GetLogicalDevice().device.createFence(&fence_create_info, nullptr, &m_frame_data[i].render_fence));
            OGFX_VK_CHECK(ogfx::VkContext::GetLogicalDevice().device.createSemaphore(&semaphore_create_info, nullptr, &m_frame_data[i].swapchain_semaphore));
            OGFX_VK_CHECK(ogfx::VkContext::GetLogicalDevice().device.createSemaphore(&semaphore_create_info, nullptr, &m_frame_data[i].render_semaphore));
        }

        compute_shader
        .AddStage(vk::ShaderStageFlagBits::eCompute, DEMO_SHADER_DIR "/test.comp.spirv")
        .Build();
    }

    void Update() override {
        m_window.Update();

        if (m_window.ShouldClose()) {
            ogfx::ShutdownEvent{}.Dispatch();
        }
    }

    void Render() override {
        if (!m_window.CanPresent()) return;

        const auto& frame_data = m_frame_data[ogfx::VkContext::GetCurrentFIF()];
        const auto& device = ogfx::VkContext::GetLogicalDevice().device;
        auto& cmd = m_cmd_bufs[ogfx::VkContext::GetCurrentFIF()];

        OGFX_VK_CHECK(device.waitForFences(1, &frame_data.render_fence, true, 1000000000));
        OGFX_VK_CHECK(device.resetFences(1, &frame_data.render_fence));
        OGFX_VK_CHECK(device.acquireNextImageKHR(m_window.GetSwapchain(), 1000000000, frame_data.swapchain_semaphore, nullptr,
            &m_swapchain_image_idx));

        auto& img = m_window.GetSwapchainImage(m_swapchain_image_idx);
        const auto& img_spec = img.GetSpec();

        cmd.BeginOTS(true);

        img.TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
            vk::PipelineStageFlagBits::eAllGraphics, vk::PipelineStageFlagBits::eAllGraphics, cmd.buf);

        vk::DescriptorImageInfo descriptor_image_info = img.GetDescriptorInfo(vk::ImageLayout::eGeneral);
        vk::WriteDescriptorSet write_descriptor = img.GetWriteDescriptorSet(0, &descriptor_image_info);
        cmd->pushDescriptorSet(vk::PipelineBindPoint::eCompute, compute_shader.GetPipelineLayout(), 0, 1, &write_descriptor);

        float v = 1.f;
        cmd->pushConstants(compute_shader.GetPipelineLayout(), vk::ShaderStageFlagBits::eAll, 0, sizeof(float), &v);
        compute_shader.Bind(cmd.buf);
        cmd->dispatch(img_spec.size.x / 8, img_spec.size.y / 8, 1);
        cmd->end();

        vk::SubmitInfo submit_info{};
        submit_info.pCommandBuffers = &cmd.buf;
        submit_info.commandBufferCount = 1;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &frame_data.swapchain_semaphore;
        vk::PipelineStageFlags dst_flags = vk::PipelineStageFlagBits::eAllGraphics;
        submit_info.pWaitDstStageMask = &dst_flags;

        ogfx::VkContext::GetLogicalDevice().SubmitGraphics(submit_info);
    }

    void RenderGUI() override {
        ImGui::ShowDemoWindow();
    }

    void Present() override {
        if (!m_window.CanPresent()) return;

        const auto& frame_data = m_frame_data[ogfx::VkContext::GetCurrentFIF()];
        auto& img = m_window.GetSwapchainImage(m_swapchain_image_idx);
        auto& cmd = m_transition_cmd_bufs[ogfx::VkContext::GetCurrentFIF()];

        cmd.BeginOTS(true);

        img.TransitionImageLayout(vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR,
            vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
            vk::PipelineStageFlagBits::eAllGraphics, vk::PipelineStageFlagBits::eAllGraphics, cmd.buf);

        cmd->end();

        vk::Semaphore render_semaphore = m_frame_data[m_swapchain_image_idx].render_semaphore;

        vk::SubmitInfo submit_info{};
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd.buf;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_semaphore;

        ogfx::VkContext::GetLogicalDevice().SubmitGraphics(submit_info, frame_data.render_fence);

        vk::SwapchainKHR swapchain = m_window.GetSwapchain();

        vk::PresentInfoKHR present_info{};
        present_info.pWaitSemaphores = &render_semaphore;
        present_info.pSwapchains = &swapchain;
        present_info.swapchainCount = 1;
        present_info.waitSemaphoreCount = 1;
        present_info.pImageIndices = &m_swapchain_image_idx;

        OGFX_VK_CHECK(ogfx::VkContext::GetLogicalDevice().SubmitPresentation(present_info));
    }

    void Shutdown() override {
        m_window.Cleanup();

        for (auto& cmd : m_cmd_bufs) cmd.Destroy();
        for (auto& cmd : m_transition_cmd_bufs) cmd.Destroy();

        for (int i = 0; i < ogfx::MAX_FRAMES_IN_FLIGHT; i++) {
            ogfx::VkContext::GetLogicalDevice().device.destroyFence(m_frame_data[i].render_fence);
            ogfx::VkContext::GetLogicalDevice().device.destroySemaphore(m_frame_data[i].swapchain_semaphore);
            ogfx::VkContext::GetLogicalDevice().device.destroySemaphore(m_frame_data[i].render_semaphore);
        }
    }

    ogfx::Image2D& GetRenderImage() {
        return m_window.GetSwapchainImage(m_swapchain_image_idx);
    }

private:
    struct FrameData {
        vk::Semaphore render_semaphore;
        vk::Semaphore swapchain_semaphore;
        vk::Fence render_fence;
    };

    uint32_t m_swapchain_image_idx = 0;

    ogfx::Shader compute_shader;

    std::array<ogfx::CommandBuffer, ogfx::MAX_FRAMES_IN_FLIGHT> m_cmd_bufs;
    std::array<ogfx::CommandBuffer, ogfx::MAX_FRAMES_IN_FLIGHT> m_transition_cmd_bufs;
    std::array<FrameData, ogfx::MAX_FRAMES_IN_FLIGHT> m_frame_data;

    ogfx::Window& m_window;
};
