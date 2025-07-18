#pragma once

#include <vulkan/vulkan.h>

#include "external/vk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"


namespace vk {

	class Swapchain;

	class Image : public utils::Move, public ptr::ToShared<Image> {
		friend class Swapchain;
	private:
		Handle<VmaAllocation> allocation{};
		Handle<VkImage> image{};

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

	public:
		ImageBuilder() = default;

		Image build();
	};
}