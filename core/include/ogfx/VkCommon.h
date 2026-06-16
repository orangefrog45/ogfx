#pragma once

#include <optional>
#include "VkContext.h"

namespace ogfx {
    uint32_t FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties);

    void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    vk::UniqueCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(vk::CommandBuffer& cmd_buf, std::optional<std::pair<vk::Semaphore, vk::PipelineStageFlags>> wait_semaphore = std::nullopt);

    void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size, vk::DeviceSize src_offset = 0, vk::DeviceSize dst_offset = 0, vk::CommandBuffer cmd = {});

    vk::Viewport CreateDefaultVkViewport(float width, float height);
}