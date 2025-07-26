#pragma once

#include <vulkan/vulkan.h>

#include "external/vk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"
#include "vk_core/CommandBuffer.h"

#include <vector>

namespace vk {

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

	public:
		Image() = default;

		inline VkImage handle() const { return *image; }
		inline VkFormat format() const { return _format; }
		inline VkExtent2D extent() const { return _extent; }
	};

	class ImageBuilder {
	public:
		using Ref = ImageBuilder&;

	private:
		VkExtent2D _extent{ 0, 0 };
		VkFormat _format{ VK_FORMAT_UNDEFINED };
		VkImageLayout _initial_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageTiling _tiling{ VK_IMAGE_TILING_OPTIMAL };

		VkImageUsageFlags _usage;
		VmaMemoryUsage _memory_usage;

		std::vector<QueueType> _queue_types;

	public:
		ImageBuilder() = default;

		inline Ref extent(VkExtent2D extent) { _extent = extent; return *this; }
		inline Ref format(VkFormat format) { _format = format; return *this; }
		inline Ref initial_layout(VkImageLayout initial_layout) { _initial_layout = initial_layout; return *this; }
		inline Ref tiling(VkImageTiling tiling) { _tiling = tiling; return *this; }

		inline Ref usage(VkImageUsageFlags usage) { _usage = usage; return *this; }
		inline Ref add_usage(VkImageUsageFlagBits usage) { _usage |= usage; return *this; }

		inline Ref memory_usage(VmaMemoryUsage memory_usage) { _memory_usage = memory_usage; return *this; }

		inline Ref queue_types(const std::vector<QueueType>& queue_types) { _queue_types = queue_types; return *this; }
		inline Ref add_queue_type(QueueType queue_type) { _queue_types.push_back(queue_type); return *this; }

		Image build() const;
	};
}