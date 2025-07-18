#include "Framebuffer.h"

#include "vk_core/Context.h"
#include <string>

namespace vk {
	Framebuffer FramebufferBuilder::build() {
		Framebuffer framebuffer;

		const auto& attachments = _render_pass->attachments();

		if (attachments.size() != _images.size()) {
			throw std::runtime_error("Framebuffer creation failed! Image count does not match attachment count");
		}

		VkExtent2D image_extent{ 0, 0 };
		bool extent_set = false;

		std::vector<ImageView> image_views{ attachments.size() };
		std::vector<VkImageView> raw_image_views{ attachments.size() };

		for (int i = 0; i < attachments.size(); i++) {
			const auto& image = _images[i];

			if (image->format() != attachments[i].format) {
				throw std::runtime_error("Framebuffer creation failed! Image format does not match attachment format");
			}

			if (!extent_set) {
				image_extent = image->extent();
				extent_set = true;
			}

			if (image->extent().width != image_extent.width || image->extent().height != image_extent.height) {
				throw std::runtime_error("Framebuffer creation failed! Image extent for attachment " + std::to_string(i) + " does not match the previous images");
			}

			VkImageAspectFlags aspect = attachments[i].type == AttachmentType::Color ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
			ImageView image_view = ImageView::create_from(image, aspect);

			raw_image_views[i] = image_view.handle();
			image_views[i] = std::move(image_view);
		}

		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = _render_pass->handle();
		framebuffer_info.attachmentCount = static_cast<uint32_t>(raw_image_views.size());
		framebuffer_info.pAttachments = raw_image_views.data();
		framebuffer_info.width = image_extent.width;
		framebuffer_info.height = image_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(Context::get()->get_device(), &framebuffer_info, nullptr, &*framebuffer.framebuffer) != VK_SUCCESS) {
			throw std::runtime_error("Framebuffer creation failed!");
		}

		framebuffer._render_pass = _render_pass;
		framebuffer._image_views = std::move(image_views);

		return framebuffer;
	}
}