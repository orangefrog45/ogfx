#pragma once

#include "ogfx/VkContext.h"

namespace ogfx {
    class CommandBuffer {
    public:
        void Init(vk::CommandBufferLevel level) {
            OGFX_ASSERT_STR(!m_initialized, "Cannot initialize a command buffer more than once.");

            m_pool_ref = VkContext::GetCommandPool();
            vk::CommandBufferAllocateInfo alloc_info{};
            alloc_info.commandPool = m_pool_ref;
            alloc_info.level = level;
            alloc_info.commandBufferCount = 1;

            OGFX_VK_CHECK(vkAllocateCommandBuffers(VkContext::GetLogicalDevice().device,
                reinterpret_cast<VkCommandBufferAllocateInfo*>(&alloc_info), reinterpret_cast<VkCommandBuffer*>(&buf)));
            m_initialized = true;
        }

        void Destroy() {
            if (m_initialized)
                vkFreeCommandBuffers(VkContext::GetLogicalDevice().device, m_pool_ref, 1, reinterpret_cast<VkCommandBuffer*>(&buf));

            m_initialized = false;
        }

        void BeginOTS(bool reset) {
            if (reset) buf.reset();

            vk::CommandBufferBeginInfo begin_info{};
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
            OGFX_VK_CHECK(buf.begin(&begin_info));
        }

        vk::CommandBuffer* operator->() {
            return &buf;
        }

        ~CommandBuffer() {
            Destroy();
        }

        vk::CommandBuffer buf;

    private:
        bool m_initialized = false;
        vk::CommandPool m_pool_ref;
    };
}