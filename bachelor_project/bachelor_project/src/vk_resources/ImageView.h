#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "Image.h"

namespace vk {
	class ImageView : public utils::Move, public ptr::ToShared<ImageView> {
	private:
		Handle<VkImageView> image_view{};
		ptr::Shared<const Image> _image{};

		VkImageAspectFlags _aspect{ 0 };

	public:
		ImageView() = default;

		inline VkImageView handle() const { return *image_view; }

		inline const ptr::Shared<const Image>& image() const { return _image; }

		inline VkFormat format() const { return _image->format(); }
		inline VkExtent2D extent() const { return _image->extent(); }
		inline VkImageAspectFlags aspect() const { return _aspect; }

		static ImageView create_from(Image&& image, VkImageAspectFlags aspect);
		static ImageView create_from(const ptr::Shared<const Image>& image, VkImageAspectFlags aspect);
	};
}