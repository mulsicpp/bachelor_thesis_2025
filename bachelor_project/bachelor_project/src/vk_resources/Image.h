#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

#include "utils/move.h"

namespace vk {

	class Image {
	public:
		VkImage image;
		VmaAllocation allocation;
		VkImageView image_view;

	public:
		Image();

	private:
		void destroy();

		inline void mark_moved() {
			image = VK_NULL_HANDLE;
		}

		inline bool was_moved() {
			return image == VK_NULL_HANDLE;
		}

		MOVE_SEMANTICS(Image)
	};
}