#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

namespace vk {

	class Image {
	public:
		VkImage image;
		VmaAllocation allocation;
		VkImageView image_view;
	};
}