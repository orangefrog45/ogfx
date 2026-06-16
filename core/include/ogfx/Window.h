#pragma once

#include "events/EventBase.h"
#include "events/EventManager.h"

#include "ogfx/VkContext.h"
#include "ogfx/resources/Image.h"
#include "ogfx/Input.h"

namespace ogfx {
    class Window {
        friend void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        friend void GlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
        friend void GlfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        friend void GlfwSizeCallback(GLFWwindow* p_glfw_window, int width, int height);
    public:
        struct ResizeEvent : ogfx::Event {
            OGFX_EVENT_CLASS(92837463462);

            uint32_t prev_width;
            uint32_t prev_height;

            uint32_t new_width;
            uint32_t new_height;

            bool prev_minimized;
            bool new_minimized;
        };

        Window(uint32_t width, uint32_t height) : m_width(width), m_height(height) {}

        void Init(const char* name);

        void CreateSwapchain();
        void DestroySwapchain();

        void Update();
        void SetCursorStyle(CursorStyle style);
        void Cleanup();

        [[nodiscard]] bool ShouldClose() const;

        [[nodiscard]] bool CanPresent() const;

        [[nodiscard]] vk::SurfaceKHR GetSurface() const {
            return m_surface;
        }

        [[nodiscard]] vk::SwapchainKHR GetSwapchain() const {
            return m_swapchain;
        }

        [[nodiscard]] GLFWwindow* GetGlfwWindow() const {
            return mp_window;
        }

        [[nodiscard]] ogfx::Image2D& GetSwapchainImage(int idx) {
            OGFX_ASSERT(idx < m_swapchain_images_wrapped.size());
            return m_swapchain_images_wrapped[idx];
        }

        [[nodiscard]] const Input& GetInput() const {
            return m_input;
        }

        [[nodiscard]] uint32_t GetWidth() const {
            return m_width;
        }

        [[nodiscard]] uint32_t GetHeight() const {
            return m_height;
        }

    private:
        inline static bool m_glfw_initialized = false;

        Input m_input;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        bool m_initialized = false;
        bool m_minimized = false;

        GLFWwindow* mp_window = nullptr;

        VkSurfaceKHR m_surface = nullptr;
        VkSwapchainKHR m_swapchain = nullptr;
        VkFormat m_swapchain_image_format = VK_FORMAT_R8G8B8A8_UNORM;

        std::vector<VkImage> m_swapchain_images;
        std::vector<ogfx::Image2D> m_swapchain_images_wrapped;
        std::vector<VkImageView> m_swapchain_image_views;
    };
}
