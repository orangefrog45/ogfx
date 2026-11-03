#pragma once

#include "ogfx/VkContext.h"

namespace ogfx {
    class Buffer {
    public:
        Buffer() = default;

        Buffer(const Buffer& other) = delete;
        Buffer& operator=(const Buffer& other) = delete;

        Buffer(Buffer&& other) noexcept :
            m_buffer(other.m_buffer),
            m_allocation(other.m_allocation),
            mp_mapped(other.mp_mapped),
            m_size(other.m_size) {
            other.m_buffer = VK_NULL_HANDLE;
            other.m_allocation = nullptr;
            other.mp_mapped = nullptr;
            other.m_size = 0;
        }

        Buffer& operator=(Buffer&& other) noexcept {
            if (this != &other) {
                Destroy();
                m_buffer = other.m_buffer;
                m_allocation = other.m_allocation;
                mp_mapped = other.mp_mapped;
                m_size = other.m_size;

                other.m_buffer = VK_NULL_HANDLE;
                other.m_allocation = nullptr;
                other.mp_mapped = nullptr;
                other.m_size = 0;
            }
            return *this;
        }

        ~Buffer() {
            Destroy();
        }

        void Create(vk::DeviceSize size, vk::BufferUsageFlags usage, bool mapped);

        void Destroy();

        // Copies size bytes from data into the buffer at offset
        void Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);

        [[nodiscard]] vk::DescriptorBufferInfo GetDescriptorInfo(
            vk::DeviceSize offset = 0,
            vk::DeviceSize range = VK_WHOLE_SIZE) const;

        static vk::WriteDescriptorSet GetWriteDescriptorSet(
            uint32_t binding, vk::DescriptorType type,
            const vk::DescriptorBufferInfo* buffer_info);

        [[nodiscard]] vk::Buffer GetBuffer() const noexcept {
            return m_buffer;
        }

        [[nodiscard]] vk::DeviceSize GetSize() const noexcept {
            return m_size;
        }

        [[nodiscard]] void* GetMappedData() const noexcept {
            return mp_mapped;
        }

    private:
        vk::Buffer m_buffer = VK_NULL_HANDLE;
        VmaAllocation m_allocation = nullptr;
        void* mp_mapped = nullptr;
        vk::DeviceSize m_size = 0;
    };
}
