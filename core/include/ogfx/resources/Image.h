#pragma once
#include <../glm/glm/glm.hpp>
#include "ogfx/VkContext.h"

namespace ogfx {
	struct Image2DSpec {
		Image2DSpec() = default;
		Image2DSpec(vk::Format fmt, vk::ImageUsageFlags use_flags, vk::ImageTiling _tiling, vk::ImageAspectFlags asp_flags, glm::uvec2 _size, unsigned mips) :
			format(fmt), usage(use_flags), tiling(_tiling), aspect_flags(asp_flags), size(_size), mip_levels(mips) {}

		vk::Format format = vk::Format::eUndefined;
		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled;
		vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
		vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eNone;
		unsigned mip_levels = 1;
		glm::uvec2 size{ 0, 0 };
	};

	class Image2D {
	public:
		Image2D() = default;

		// Creates an Image2D "wrapper" around an existing vkImage, this class will not handle deallocation if using this constructor
		Image2D(vk::Image image, Image2DSpec spec) : m_image(image), m_owns_image(false) {
			SetSpec(spec);
			CreateImageView();
		}

		Image2D(const Image2DSpec& spec);

		Image2D(const Image2D& other) = delete;

		Image2D(Image2D&& other) noexcept :
			m_spec(other.m_spec),
			m_sampler(std::move(other.m_sampler)),
			m_image(other.m_image),
			m_view(std::move(other.m_view)),
			m_allocation(other.m_allocation),
			m_owns_image(other.m_owns_image) {
			other.m_image = VK_NULL_HANDLE;
			other.m_allocation = nullptr;
		}

		Image2D& operator=(const Image2D& other) = delete;

		Image2D& operator=(Image2D&& other) noexcept {
			if (this != &other) {
				DestroyImage();
				m_spec = other.m_spec;
				m_sampler = std::move(other.m_sampler);
				m_image = other.m_image;
				m_view = std::move(other.m_view);
				m_allocation = other.m_allocation;
				m_owns_image = other.m_owns_image;

				other.m_image = VK_NULL_HANDLE;
				other.m_allocation = nullptr;
			}
			return *this;
		}

		~Image2D() {
			if (m_owns_image) {
				DestroyImage();
				return;
			}

			if (m_sampler) m_sampler.reset();
			if (m_view) m_view.reset();
		}

		void SetSpec(const Image2DSpec& spec) {
			OGFX_ASSERT(spec.format != vk::Format::eUndefined);
			m_spec = spec;
		}

		const Image2DSpec& GetSpec() {
			return m_spec;
		}

		void CreateImage(VmaAllocationCreateFlags flags=0);

		void DestroyImage();

		void GenerateMipmaps(vk::ImageLayout start_layout);

		// Returns an image view created with format 'fmt' (if provided) or (if not provided) image.m_spec.format
		[[nodiscard]] static vk::UniqueImageView CreateImageView(const Image2D& image, std::optional<vk::Format> fmt = std::nullopt);

		void BlitTo(Image2D& dst, unsigned dst_mip, unsigned src_mip, vk::ImageLayout start_src_layout, vk::ImageLayout start_dst_layout,
			vk::ImageLayout final_src_layout, vk::ImageLayout final_dst_layout, vk::Filter filter, 
			std::optional<vk::Semaphore> wait_semaphore = std::nullopt, std::optional<vk::CommandBuffer> cmd_buf = std::nullopt);

		void TransitionImageLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout,
			vk::CommandBuffer buf = {}, unsigned mip_level = 0, unsigned level_count = 1);

		void TransitionImageLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout, 
			vk::AccessFlags src_access, vk::AccessFlags dst_access, vk::PipelineStageFlags src_stage, vk::PipelineStageFlags dst_stage,  
			vk::CommandBuffer buf = {}, unsigned mip_level = 0, unsigned level_count = 1);

		void PipelineBarrier(vk::ImageLayout layout,
			vk::AccessFlags src_access, vk::AccessFlags dst_access, vk::PipelineStageFlags src_stage, vk::PipelineStageFlags dst_stage,
			vk::CommandBuffer buf = {}, unsigned mip_level = 0, unsigned level_count = 1);

		vk::RenderingAttachmentInfo CreateAttachmentInfo(vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op, vk::ImageLayout layout, vk::ClearColorValue clear_val = {0.f, 0.f, 0.f, 0.f});

		vk::ImageSubresourceRange GetFullSubresourceRange(vk::ImageAspectFlags aspect_mask) const;
		
		vk::DescriptorImageInfo GetDescriptorInfo(vk::ImageLayout layout) const;

		vk::WriteDescriptorSet GetWriteDescriptorSet(uint32_t binding, vk::DescriptorImageInfo* image_info) const;

		vk::ImageView GetImageView() const {
			return *m_view;
		}

		vk::Sampler GetSampler() const {
			return *m_sampler;
		}

		vk::Image GetImage() const noexcept {
			return m_image;
		}

		bool ImageIsCreated() const {
			return static_cast<bool>(m_image);
		}

		vk::DeviceMemory GetMemory();

	protected:
		void CreateSampler();

		void CreateImageView();

		void DestroyImageView() {
			m_view.release();
		}

		void DestroySampler() {
			m_sampler.release();
		}

		Image2DSpec m_spec;

		vk::UniqueSampler m_sampler;
		vk::Image m_image = VK_NULL_HANDLE;
		vk::UniqueImageView m_view;
		VmaAllocation m_allocation = nullptr;

		bool m_owns_image = true;
	};

	// An image that always resizes itself to the current active window dimensions
	// class FullscreenImage2D : public Image2D {
	// public:
	// 	// Initial layout is the layout this image is transitioned to whenever the image is (re)created
	// 	explicit FullscreenImage2D(vk::ImageLayout initial_layout);
	// private:
	// 	const vk::ImageLayout m_initial_layout;
	//
	// 	std::optional<SwapchainInvalidateEvent> m_swapchain_invalidate_event_data;
	// 	EventListener m_frame_start_listener;
	// 	EventListener m_swapchain_invalidate_listener;
	// };


}