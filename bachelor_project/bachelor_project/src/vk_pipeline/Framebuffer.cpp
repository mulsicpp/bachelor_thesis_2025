#include "Framebuffer.h"

#include "vk_core/Context.h"
#include <string>

namespace vk {

	void Framebuffer::cmd_begin_pass(ReadyCommandBuffer cmd_buffer, const PassBeginInfo& info) {
		VkRenderPassBeginInfo pass_begin_info{};
		pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass_begin_info.renderPass = _render_pass->handle();
		pass_begin_info.framebuffer = *framebuffer;
		pass_begin_info.renderArea.offset = info.render_area.offset;


		auto extent = info.render_area.extent;
		if (extent.width == UINT32_MAX) {
			extent.width = _extent.width - info.render_area.offset.x;
		}

		if (extent.height == UINT32_MAX) {
			extent.height = _extent.height - info.render_area.offset.y;
		}

		pass_begin_info.renderArea.extent = extent;

		std::vector<VkClearValue> clear_values{ info.clear_values.size() };
		for (int i = 0; i < clear_values.size(); i++) {
			clear_values[i] = info.clear_values[i].get();
		}

		pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		pass_begin_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(cmd_buffer.handle(), &pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

		_render_pass->_active = true;
		_render_pass->_begin_info = info;
	}

	void Framebuffer::cmd_end_pass(ReadyCommandBuffer cmd_buffer) {
		vkCmdEndRenderPass(cmd_buffer.handle());

		_render_pass->_active = false;
	}

	Framebuffer FramebufferBuilder::build() {
		Framebuffer framebuffer;

		const auto& attachments = _render_pass->attachments();

		if (attachments.size() != _images.size()) {
			throw std::runtime_error("Framebuffer creation failed! Image count does not match attachment count");
		}

		if (attachments.size() == 0) {
			throw std::runtime_error("Framebuffer creation failed! Framebuffer needs at least one attachment");
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
		framebuffer._extent = image_extent;

		return framebuffer;
	}
}