#pragma once

#include <vulkan/vulkan.h>

#include "external/vk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"


namespace vk {

	class Image : public utils::Move, public ptr::ToShared<Image> {
	public:
		Handle<VmaAllocation> allocation{};
		Handle<VkImage> image{};
		Handle<VkImageView> image_view{};

		VkFormat format{ VK_FORMAT_UNDEFINED };
		VkExtent2D extent{ 0, 0 };

	public:
		Image() = default;
	};

	class ImageBuilder {
	public:

	public:
		ImageBuilder() = default;

		Image build();
	};
}