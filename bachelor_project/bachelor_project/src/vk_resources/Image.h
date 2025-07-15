#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

#include "utils/move.h"

#include "Format.h"

namespace vk {

	struct ImageExtent {
		uint32_t width;
		uint32_t height;
	};

	class Image {
	public:
		VkImage image;
		VmaAllocation allocation;
		VkImageView image_view;

		Format format;
		ImageExtent extent;

	public:
		Image();

	private:
		void destroy();

		MOVE_SEMANTICS_VK_HANDLE(Image, image)
	};

	class ImageBuilder {
	public:

	public:
		ImageBuilder() = default;

		Image build();
	};
}