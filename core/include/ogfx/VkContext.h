#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <optional>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>
#include <VkBootstrap.h>

#include "ogfx/util/VkUtil.h"

namespace ogfx {
	using FrameInFlightIndex = uint8_t;

	// THIS SHOULD NEVER BE ABOVE 4
	constexpr FrameInFlightIndex MAX_FRAMES_IN_FLIGHT = 3;

	class LogicalDevice {
	public:
		vk::Device device;
		vkb::Device vkb_device;

		void SubmitGraphics(const vk::SubmitInfo& info, std::optional<vk::Fence> fence = std::nullopt) {
			m_graphics_mux.lock();
			m_graphics_queue.submit(info, fence.has_value() ? fence.value() : nullptr);
			m_graphics_mux.unlock();
		}

		void GraphicsQueueWaitIdle() {
			m_graphics_mux.lock();
			m_graphics_queue.waitIdle();
			m_graphics_mux.unlock();
		}

		[[nodiscard]] vk::Result SubmitPresentation(const vk::PresentInfoKHR& info) {
			m_present_mux.lock();
			auto res = m_presentation_queue.presentKHR(info);
			m_present_mux.unlock();
			return res;
		}

		[[nodiscard]] uint32_t GetGraphicsQueueIdx() const {
			return m_graphics_queue_idx;
		}

		[[nodiscard]] uint32_t GetPresentationQueueIdx() const {
			return m_presentation_queue_idx;
		}

		[[nodiscard]] vk::Queue GetGraphicsQueue() const {
			return m_graphics_queue;
		}

	private:
		std::mutex m_graphics_mux;
		std::mutex m_present_mux;

		vk::Queue m_graphics_queue;
		vk::Queue m_presentation_queue;

		uint32_t m_graphics_queue_idx = 0;
		uint32_t m_presentation_queue_idx = 0;

		friend class VkContext;
	};

	struct PhysicalDevice {
		PhysicalDevice() {
			buffer_properties.pNext = &accel_properties;
			accel_properties.pNext = &ray_properties;
		}

		vk::PhysicalDevice device;
		vkb::PhysicalDevice vkb_device;
		vk::PhysicalDeviceProperties properties;
		vk::PhysicalDeviceDescriptorBufferPropertiesEXT buffer_properties;
		vk::PhysicalDeviceAccelerationStructurePropertiesKHR accel_properties;
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR ray_properties;
	};

	class VkContext {
	public:
		static VkContext& Get() {
			static VkContext s_instance;
			return s_instance;
		}

		static void CreateInstance(const char* app_name) {
			OGFX_ASSERT(!Get().m_instance);
			Get().ICreateInstance(app_name);
		}

		static void PickPhysicalDevice(std::optional<vk::SurfaceKHR> surface = std::nullopt) {
			Get().IPickPhysicalDevice(surface);
		}

		static void CreateLogicalDevice() {
			Get().ICreateLogicalDevice();
		}


		static vk::CommandPool GetCommandPool() {
			return Get().m_cmd_pools[std::this_thread::get_id()][GetCurrentFIF()];
		}

		static FrameInFlightIndex GetCurrentFIF() {
			return Get().m_current_frame_idx % MAX_FRAMES_IN_FLIGHT;
		}

		static uint64_t GetCurrentFrameIdx() {
			return Get().m_current_frame_idx;
		}

		static void CreateCommandPools();

		static void InitVMA() {
			Get().I_InitVMA();
		}

		static VmaAllocator& GetAllocator() {
			return Get().m_allocator;
		}

		static vk::Instance& GetInstance() {
			return Get().m_instance;
		}

		static PhysicalDevice& GetPhysicalDevice() {
			return Get().m_physical_device;
		}

		static LogicalDevice& GetLogicalDevice() {
			return Get().m_device;
		}

		static void NextFrame() {
			Get().m_current_frame_idx++;
		}

		static void Cleanup() {
			Get().ICleanup();
		}

	private:
		VkContext(const VkContext& other) = delete;
		VkContext()=default;

		void I_InitVMA() {
			VmaVulkanFunctions vulkan_functions{};
			vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
			vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

			VmaAllocatorCreateInfo alloc_info{};
			alloc_info.device = m_device.device;
			alloc_info.physicalDevice = m_physical_device.device;
			alloc_info.instance = m_instance;
			alloc_info.pVulkanFunctions = &vulkan_functions;
			alloc_info.flags = VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			OGFX_VK_CHECK(vmaCreateAllocator(&alloc_info, &m_allocator));
		}

		void ICreateInstance(const char* app_name);

		void IPickPhysicalDevice(std::optional<vk::SurfaceKHR> surface);

		void ICreateLogicalDevice();

		void ICleanup();

		uint64_t m_current_frame_idx = 0;

		vk::Instance m_instance;
		vkb::Instance m_vkb_instance;

		PhysicalDevice m_physical_device;
		LogicalDevice m_device;

		VkDebugUtilsMessengerEXT m_debug_messenger;

		std::unordered_map<std::thread::id, std::array<vk::CommandPool, MAX_FRAMES_IN_FLIGHT>> m_cmd_pools;
		VmaAllocator m_allocator{};

		friend class App;
	};
}
