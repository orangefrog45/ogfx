#include "ogfx/pch.h"

#define VMA_IMPLEMENTATION
#include "ogfx/VkContext.h"
#include "ogfx/multithreading/JobSystem.h"

#include <VkBootstrap.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

using namespace ogfx;

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::string type;
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) type = "GENERAL";
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) type = "VALIDATION";
    else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) type = "PERFORMANCE";

    std::string message = std::format("[VK][{}]: {}", type, pCallbackData->pMessage);

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        OGFX_CORE_TRACE(message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        OGFX_CORE_INFO(message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        OGFX_CORE_WARN(message);
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        OGFX_CORE_ERROR(message);
    }

    return VK_FALSE;
}

void VkContext::IPickPhysicalDevice(std::optional<vk::SurfaceKHR> surface) {
    VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
    features.dynamicRendering = true;
    features.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;
    features12.timelineSemaphore = true;

    vkb::PhysicalDeviceSelector selector{m_vkb_instance};

    vk::PhysicalDeviceShaderObjectFeaturesEXT shader_object_features{};
    shader_object_features.shaderObject = true;

    selector
    .set_minimum_version(1, 4)
    .set_required_features_13(features)
    .set_required_features_12(features12)
    .add_required_extension(VK_EXT_SHADER_OBJECT_EXTENSION_NAME)
    .add_required_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)
    .add_required_extension_features(shader_object_features);

    if (surface.has_value())
        selector.set_surface(surface.value());
    else
        selector.defer_surface_initialization();

    m_physical_device.vkb_device = selector.select().value();
    m_physical_device.device = m_physical_device.vkb_device.physical_device;
}

void VkContext::ICreateLogicalDevice() {
    vkb::DeviceBuilder device_builder{m_physical_device.vkb_device};
    m_device.vkb_device = device_builder.build().value();
    m_device.device = m_device.vkb_device.device;
    volkLoadDevice(m_device.device);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device.device);

    m_device.m_graphics_queue = m_device.vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_device.m_graphics_queue_idx = m_device.vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    auto vkb_present_queue = m_device.vkb_device.get_queue(vkb::QueueType::present);
    if (vkb_present_queue.has_value()) {
        m_device.m_presentation_queue = vkb_present_queue.value();
        m_device.m_presentation_queue_idx = m_device.vkb_device.get_queue_index(vkb::QueueType::present).value();
    }
}

void VkContext::CreateCommandPools() {
    auto num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread::id> thread_ids = JobSystem::GetThreadIDs();

    vk::CommandPoolCreateInfo pool_info{};
    pool_info.queueFamilyIndex = Get().m_device.GetGraphicsQueueIdx();
    pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    for (unsigned i = 0; i < num_threads; i++) {
        for (FrameInFlightIndex fif = 0; fif < MAX_FRAMES_IN_FLIGHT; fif++) {
            vk::CommandPool pool;
            OGFX_VK_CHECK(vkCreateCommandPool(GetLogicalDevice().device,
                reinterpret_cast<VkCommandPoolCreateInfo*>(&pool_info),
                nullptr,
                reinterpret_cast<VkCommandPool*>(&pool)));

            Get().m_cmd_pools[thread_ids[i]][fif] = pool;
        }
    }
}

void VkContext::ICreateInstance(const char* app_name) {
    OGFX_VK_CHECK(volkInitialize());
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    std::vector<const char*> extensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
        "VK_KHR_win32_surface",
#endif
#if defined (__APPLE__)
        "VK_MVK_macos_surface"
#endif
#if defined (__linux__)
        "VK_KHR_xcb_surface"
#endif
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    vkb::InstanceBuilder builder;

    auto inst_ret = builder.set_app_name(app_name)
    .request_validation_layers(true)
    .set_debug_callback(VulkanDebugCallback)
    .require_api_version(1, 4, 1)
    .enable_extensions(extensions.size(), extensions.data())
    .build();

    if (inst_ret.has_value()) {
        m_vkb_instance = inst_ret.value();
    } else {
        OGFX_CORE_ERROR("Failed to create instance:");
        for (const auto& reason : inst_ret.detailed_failure_reasons()) {
            OGFX_CORE_ERROR(reason);
        }

        OGFX_BREAKPOINT;
    }

    m_instance = m_vkb_instance.instance;
    m_debug_messenger = m_vkb_instance.debug_messenger;

	volkLoadInstance(m_instance);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);
}

void VkContext::ICleanup() {
   vmaDestroyAllocator(m_allocator);

    for (auto& [thread_id, cmd_pools] : m_cmd_pools) {
        for (FrameInFlightIndex i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyCommandPool(m_device.device, cmd_pools[i], nullptr);
        }
    }

    vkDestroyDevice(m_device.device, nullptr);
    vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
    vkDestroyInstance(m_instance, nullptr);
}
