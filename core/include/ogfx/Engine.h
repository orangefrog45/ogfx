#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>
#include <VkBootstrap.h>
#include <../imgui/imgui.h>
#include <../imgui/backends/imgui_impl_glfw.h>
#include <../imgui/backends/imgui_impl_vulkan.h>

#include "Gui.h"
#include "layers/LayerStack.h"
#include "ogfx/multithreading/JobSystem.h"

#include "ogfx/events/EventManager.h"
#include "ogfx/events/EventListener.h"
#include "ogfx/util/Logger.h"
#include "ogfx/util/VkUtil.h"
#include "ogfx/VkContext.h"
#include "ogfx/Window.h"
#include "ogfx/VkCommands.h"
#include "ogfx/resources/Image.h"
#include "ogfx/util/Util.h"

namespace ogfx {
    struct ShutdownEvent : Event {
        OGFX_EVENT_CLASS(273791929378);
    };

    struct ImGuiRenderData {
        Image2D* p_output_image = nullptr;
        vk::ImageLayout output_image_layout = vk::ImageLayout::eUndefined;
    };

    class Engine {
    public:
        struct InitData {
            // OPTIONAL - Application window
            Window* p_window = nullptr;

            // OPTIONAL - Function invoked each frame that returns ImGuiRenderData for that frame
            std::function<ImGuiRenderData()> imgui_rd_func = nullptr;
        };

        void Init(const InitData& init, const char* app_name) {
            m_init_data = init;

            JobSystem::Init();
            VkContext::CreateInstance(app_name);

            if (init.p_window) init.p_window->Init(app_name);

            VkContext::PickPhysicalDevice(init.p_window ? std::make_optional(init.p_window->GetSurface()) : std::nullopt);
            VkContext::CreateLogicalDevice();
            VkContext::InitVMA();
            VkContext::CreateCommandPools();

            if (init.p_window) {
                init.p_window->CreateSwapchain();
                m_imgui_descriptor_pool = InitImGui(*init.p_window);
                for (auto& cmd : m_imgui_cmd_bufs) cmd.Init(vk::CommandBufferLevel::ePrimary);
            }

            m_shutdown_listener = {[&](auto&) {m_running = false;}, true};
            m_initialized = true;
            layers.Init();
        }

        void MainLoop() {
            m_running = true;
            while (m_running) {
                layers.Update();
                layers.Render();

                if (m_init_data.p_window && m_init_data.imgui_rd_func && m_init_data.p_window->CanPresent())
                    RenderImGui(m_init_data.imgui_rd_func());

                layers.Present();
                VkContext::NextFrame();
            }

            VkContext::GetLogicalDevice().GraphicsQueueWaitIdle();
        }

        void Cleanup() {
            if (!m_initialized) return;

            auto& device = VkContext::GetLogicalDevice();
            device.GraphicsQueueWaitIdle();

            layers.Shutdown();

            if (m_init_data.p_window) {
                for (auto& cmd : m_imgui_cmd_bufs) cmd.Destroy();
                ImGui_ImplVulkan_Shutdown();
                device.device.destroyDescriptorPool(m_imgui_descriptor_pool);
            }

            VkContext::Cleanup();
            JobSystem::Shutdown();

            m_shutdown_listener.Deregister();

            m_initialized = false;
        }

        LayerStack layers;
    private:
        void RenderImGui(const ImGuiRenderData& rd) {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            layers.RenderGUI();

            ImGui::Render();

            auto& cmd = m_imgui_cmd_bufs[ogfx::VkContext::GetCurrentFIF()];
            cmd.BeginOTS(true);
            RecordRenderImGui(*rd.p_output_image, cmd.buf);
            cmd->end();

            vk::SubmitInfo submit_info{};
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cmd.buf;
            ogfx::VkContext::GetLogicalDevice().SubmitGraphics(submit_info);
        }

        std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> m_imgui_cmd_bufs;

        EventListener<ShutdownEvent> m_shutdown_listener;
        bool m_running = false;
        bool m_initialized = false;

        InitData m_init_data;
        vk::DescriptorPool m_imgui_descriptor_pool;
    };
}
