#include "ogfx/pch.h"
#include "ogfx/Window.h"

using namespace ogfx;

namespace ogfx {
    void GlfwKeyCallback([[maybe_unused]] GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
        if (action != GLFW_RELEASE && action != GLFW_PRESS)
            return; // "Hold" state tracked in input, not here

        auto* p_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        p_window->m_input.m_key_states[static_cast<Key>(key)] = static_cast<InputType>(action);
    }

    void GlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        auto* p_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        p_window->m_input.m_scroll_state.active = true;
        p_window->m_input.m_scroll_state.offset = { (float)xoffset, (float)yoffset };
    }

    void GlfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        auto* p_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        p_window->m_input.m_mouse_position = { (int)xpos, (int)ypos };
    }

    void GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, [[maybe_unused]] int mods) {
        if (action != GLFW_RELEASE && action != GLFW_PRESS)
            return;

        auto* p_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
        p_window->m_input.m_mouse_states[static_cast<MouseButton>(button)] = static_cast<InputType>(action);
    }

    void GlfwSizeCallback(GLFWwindow* p_glfw_window, [[maybe_unused]] int width, [[maybe_unused]] int height) {
        VkContext::GetLogicalDevice().GraphicsQueueWaitIdle();

        auto* p_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(p_glfw_window));

        Window::ResizeEvent _event;
        _event.prev_width = p_window->m_width;
        _event.prev_height = p_window->m_height;
        _event.prev_minimized = p_window->m_minimized;
        _event.new_width = width;
        _event.new_height = height;

        p_window->DestroySwapchain();

        if (width == 0 || height == 0) {
            p_window->m_minimized = true;
            _event.new_minimized = true;
        } else {
            p_window->m_minimized = false;
            _event.new_minimized = false;
            p_window->m_width = width;
            p_window->m_height = height;
            p_window->CreateSwapchain();
        }

        _event.Dispatch();
    }
}


void Window::Init(const char* name) {
    if (!m_glfw_initialized) {
        OGFX_ASSERT_STR(glfwInit() == GLFW_TRUE, "GLFW failed to initialize");
    }

    OGFX_ASSERT_STR(!m_initialized, "Cannot initialize a window more than once.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    mp_window = glfwCreateWindow(m_width, m_height, name, nullptr, nullptr);
    OGFX_ASSERT(mp_window);

    glfwSetWindowUserPointer(mp_window, this);
    glfwSetKeyCallback(mp_window, GlfwKeyCallback);
    glfwSetScrollCallback(mp_window, GlfwScrollCallback);
    glfwSetCursorPosCallback(mp_window, GlfwCursorPosCallback);
    glfwSetMouseButtonCallback(mp_window, GlfwMouseButtonCallback);
    glfwSetWindowSizeCallback(mp_window, GlfwSizeCallback);

    if (!mp_window) {
        OGFX_CORE_CRITICAL("Failed to create window.");
        glfwTerminate();
        return;
    }

    glfwCreateWindowSurface(VkContext::GetInstance(), mp_window, nullptr, &m_surface);

    m_initialized = true;
}

void Window::CreateSwapchain() {
    vkb::SwapchainBuilder swapchain_builder{VkContext::GetLogicalDevice().vkb_device, m_surface};
    vkb::Swapchain vkb_swapchain = swapchain_builder
    .set_desired_format(VkSurfaceFormatKHR{.format = m_swapchain_image_format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    .set_desired_extent(m_width, m_height)
    .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
    .build()
    .value();

    m_swapchain = vkb_swapchain.swapchain;
    m_swapchain_images = vkb_swapchain.get_images().value();
    m_swapchain_image_views = vkb_swapchain.get_image_views().value();
    m_swapchain_image_format = vkb_swapchain.image_format;

    Image2DSpec swapchain_spec{};
    swapchain_spec.format = (vk::Format)m_swapchain_image_format;
    swapchain_spec.size = {m_width, m_height};
    swapchain_spec.usage = vk::ImageUsageFlagBits::eTransferDst;
    swapchain_spec.aspect_flags = vk::ImageAspectFlagBits::eColor;

    for (const auto& img : m_swapchain_images) {
        m_swapchain_images_wrapped.emplace_back((vk::Image)img, swapchain_spec);
    }
}

void Window::DestroySwapchain() {
    if (!m_swapchain) return;

    auto& device = VkContext::GetLogicalDevice().device;

    device.destroySwapchainKHR(m_swapchain);
    m_swapchain = nullptr;

    for (size_t i = 0; i < m_swapchain_image_views.size(); i++) {
        device.destroyImageView(m_swapchain_image_views[i]);
    }

    m_swapchain_images.clear();
    m_swapchain_images_wrapped.clear();
}

void Window::Update() {
    // This order ensures that the PRESS state is active for one frame
    m_input.Update();
    glfwPollEvents();
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(mp_window);
}

bool Window::CanPresent() const {
    return !m_minimized && m_width > 0 && m_height > 0;
}

void Window::SetCursorStyle(CursorStyle style) {
    static GLFWcursor* cursors[static_cast<size_t>(CursorStyle::NUM_CURSORS)] = { 0 };

    if (!cursors[0]) {
        cursors[static_cast<unsigned>(CursorStyle::ARROW)] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        cursors[static_cast<unsigned>(CursorStyle::I_BEAM)] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        cursors[static_cast<unsigned>(CursorStyle::VRESIZE)] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        cursors[static_cast<unsigned>(CursorStyle::HRESIZE)] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        cursors[static_cast<unsigned>(CursorStyle::HAND)] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    }

    glfwSetCursor(mp_window, cursors[static_cast<unsigned>(style)]);
}

void Window::Cleanup() {
    m_swapchain_images_wrapped.clear();
    vkDestroySwapchainKHR(VkContext::GetLogicalDevice().device, m_swapchain, nullptr);

    for (size_t i = 0; i < m_swapchain_image_views.size(); ++i) {
        vkDestroyImageView(VkContext::GetLogicalDevice().device, m_swapchain_image_views[i], nullptr);
    }

    vkDestroySurfaceKHR(VkContext::GetInstance(), m_surface, nullptr);
    glfwDestroyWindow(mp_window);

    m_initialized = false;
}
