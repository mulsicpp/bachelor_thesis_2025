#pragma once

#include <vulkan/vulkan.h>

#include "external/vk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"


namespace vk {

	class SwapchainDeprecated;
	class Swapchain;

	class Image : public utils::Move, public ptr::ToShared<Image> {
		friend class SwapchainDeprecated;
		friend class SwapchainBuilder;
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

	public:
		ImageBuilder() = default;

		Image build();
	};
}