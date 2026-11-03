#include "ogfx/pch.h"
#include "ogfx/resources/Buffer.h"
#include "ogfx/VkCommon.h"

using namespace ogfx;

void Buffer::Create(vk::DeviceSize size, vk::BufferUsageFlags usage, bool mapped) {
    OGFX_ASSERT(!m_buffer);
    OGFX_ASSERT(size > 0);

    vk::BufferCreateInfo buffer_info{};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = vk::SharingMode::eExclusive;

    auto buf_info = static_cast<VkBufferCreateInfo>(buffer_info);
    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

    if (mapped) {
        alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }

    VmaAllocationInfo allocation_info{};
    OGFX_VK_CHECK(
        vmaCreateBuffer(VkContext::GetAllocator(), &buf_info, &alloc_info,
            reinterpret_cast<VkBuffer*>(&m_buffer), &m_allocation, &allocation_info));

    if (mapped) mp_mapped = allocation_info.pMappedData;

    m_size = size;
}

void Buffer::Destroy() {
    if (m_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(VkContext::GetAllocator(), m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = nullptr;
        mp_mapped = nullptr;
        m_size = 0;
    }
}

void Buffer::Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset) {
    OGFX_ASSERT(mp_mapped);
    OGFX_ASSERT(offset + size <= m_size);

    std::memcpy(static_cast<std::byte*>(mp_mapped) + offset, data, size);
    // No-op if the memory is host coherent
    OGFX_VK_CHECK(
        vmaFlushAllocation(VkContext::GetAllocator(), m_allocation, offset, size));
}

vk::DescriptorBufferInfo Buffer::GetDescriptorInfo(vk::DeviceSize offset,
                                                   vk::DeviceSize range) const {
    vk::DescriptorBufferInfo descriptor_buffer_info{};
    descriptor_buffer_info.buffer = m_buffer;
    descriptor_buffer_info.offset = offset;
    descriptor_buffer_info.range = range;

    return descriptor_buffer_info;
}

vk::WriteDescriptorSet Buffer::GetWriteDescriptorSet(
    uint32_t binding, vk::DescriptorType type,
    const vk::DescriptorBufferInfo* buffer_info) {
    vk::WriteDescriptorSet write_descriptor;
    write_descriptor.descriptorType = type;
    write_descriptor.descriptorCount = 1;
    write_descriptor.pBufferInfo = buffer_info;
    write_descriptor.dstBinding = binding;

    return write_descriptor;
}
