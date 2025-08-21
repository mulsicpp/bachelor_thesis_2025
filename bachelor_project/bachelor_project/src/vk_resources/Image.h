#pragma once

#include "external/volk.h"

#include "external/volk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"
#include "vk_core/CommandBuffer.h"

#include "Buffer.h"

#include <vector>

namespace vk {

	enum class ImageState {
		Undefined,
		TransferSrc,
		TransferDst,
		ColorOutput
	};

	class SwapchainDeprecated;
	class Swapchain;

	class ImageBuilder;

	class Image : public utils::Move, public ptr::ToShared<Image> {
		friend class SwapchainDeprecated;
		friend class Swapchain;
		friend class ImageBuilder;
	private:
		Handle<VmaAllocation> allocation{};
		Handle<ImageHandle> image{};

		VkFormat _format{ VK_FORMAT_UNDEFINED };
		VkExtent2D _extent{ 0, 0 };
		VkImageAspectFlags _aspect{ VK_IMAGE_ASPECT_COLOR_BIT };

	public:
		Image() = default;

		inline VkImage handle() const { return *image; }

		inline VkFormat format() const { return _format; }
		inline VkExtent2D extent() const { return _extent; }
		inline VkImageAspectFlags aspect() const { return _aspect; }

		void cmd_transition(ReadyCommandBuffer cmd_buffer, ImageState src_state, ImageState dst_state);

		void cmd_load(ReadyCommandBuffer cmd_buffer, Buffer* buffer, const std::vector<VkBufferImageCopy>& copy_regions = {});
		void cmd_store(ReadyCommandBuffer cmd_buffer, Buffer* buffer, const std::vector<VkBufferImageCopy>& copy_regions = {});

	};


	class ImageBuilder {
	public:
		using Ref = ImageBuilder&;

	private:
		VkExtent2D _extent{ 0, 0 };
		VkFormat _format{ VK_FORMAT_UNDEFINED };
		VkImageLayout _initial_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageTiling _tiling{ VK_IMAGE_TILING_OPTIMAL };

		VkImageAspectFlags _aspect{ VK_IMAGE_ASPECT_COLOR_BIT };
		VkImageUsageFlags _usage{ 0 };
		VmaMemoryUsage _memory_usage{ VMA_MEMORY_USAGE_UNKNOWN };

		std::vector<QueueType> _queue_types{};

	public:
		ImageBuilder() = default;

		inline Ref extent(VkExtent2D extent) { _extent = extent; return *this; }
		inline Ref format(VkFormat format) { _format = format; return *this; }
		inline Ref initial_layout(VkImageLayout initial_layout) { _initial_layout = initial_layout; return *this; }
		inline Ref tiling(VkImageTiling tiling) { _tiling = tiling; return *this; }

		inline Ref aspect(VkImageAspectFlags aspect) { _aspect = aspect; return *this; }

		inline Ref usage(VkImageUsageFlags usage) { _usage = usage; return *this; }
		inline Ref add_usage(VkImageUsageFlagBits usage) { _usage |= usage; return *this; }

		inline Ref memory_usage(VmaMemoryUsage memory_usage) { _memory_usage = memory_usage; return *this; }

		inline Ref queue_types(const std::vector<QueueType>& queue_types) { _queue_types = queue_types; return *this; }
		inline Ref add_queue_type(QueueType queue_type) { _queue_types.push_back(queue_type); return *this; }

		Image build() const;
	};
}