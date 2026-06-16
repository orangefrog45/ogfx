#include "ogfx/VkCommon.h"

using namespace ogfx;

uint32_t ogfx::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) {
    auto phys_mem_properties = VkContext::GetPhysicalDevice().device.getMemoryProperties();

    for (uint32_t i = 0; i < phys_mem_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) &&
            (static_cast<uint32_t>(phys_mem_properties.memoryTypes[i].propertyFlags) & static_cast<uint32_t>(properties)) == static_cast<uint32_t>(properties)) {
            return i;
            }
    }

    OGFX_ASSERT_STR(false, "Failed to find memory type");
    return 0;
}

void ogfx::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
    auto cmd_buf = BeginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0; // pixels tightly packed
    region.bufferImageHeight = 0; // pixels tightly packed
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{ 0, 0, 0 };
    region.imageExtent = vk::Extent3D{ width, height, 1 };

    cmd_buf->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);

    EndSingleTimeCommands(*cmd_buf);
}

vk::UniqueCommandBuffer ogfx::BeginSingleTimeCommands() {
	vk::CommandBufferAllocateInfo alloc_info{};
	alloc_info.level = vk::CommandBufferLevel::ePrimary;
	alloc_info.commandPool = VkContext::GetCommandPool();
	alloc_info.commandBufferCount = 1;

	vk::UniqueCommandBuffer cmd_buf = std::move(VkContext::GetLogicalDevice().device.allocateCommandBuffersUnique(alloc_info)[0]);

	vk::CommandBufferBeginInfo begin_info{};
	begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	cmd_buf->begin(begin_info);

	return std::move(cmd_buf);
}

void ogfx::EndSingleTimeCommands(vk::CommandBuffer& cmd_buf, std::optional<std::pair<vk::Semaphore, vk::PipelineStageFlags>> wait_semaphore_stage) {
	cmd_buf.end();

	vk::SubmitInfo submit_info{};
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd_buf;
	if (wait_semaphore_stage.has_value()) {
		submit_info.pWaitSemaphores = &wait_semaphore_stage.value().first;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitDstStageMask = &wait_semaphore_stage.value().second;
	}

	auto& device = VkContext::GetLogicalDevice();

	device.SubmitGraphics(submit_info);
	device.GraphicsQueueWaitIdle();
}

void ogfx::CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size, vk::DeviceSize src_offset, vk::DeviceSize dst_offset, vk::CommandBuffer cmd) {
	bool temp_cmd = !cmd;
	vk::UniqueCommandBuffer temp;
	if (temp_cmd) {
		temp = BeginSingleTimeCommands();
		cmd = *temp;
	}

	vk::BufferCopy copy_region{};
	copy_region.srcOffset = src_offset;
	copy_region.dstOffset = dst_offset;
	copy_region.size = size;

	cmd.copyBuffer(src, dst, copy_region);

	if (temp_cmd)
		EndSingleTimeCommands(cmd);
}

vk::Viewport ogfx::CreateDefaultVkViewport(float width, float height) {
	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	return viewport;
}