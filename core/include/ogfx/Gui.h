#pragma once

#include "VkContext.h"
#include "resources/Image.h"

namespace ogfx {
    [[nodiscard]] vk::DescriptorPool InitImGui(class Window& window);
    void RecordRenderImGui(ogfx::Image2D& img, vk::CommandBuffer& cmd);
}