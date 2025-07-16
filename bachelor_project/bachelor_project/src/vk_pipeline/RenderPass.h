#pragma once

#include <vulkan/vulkan.h>


#include "utils/move.h"

#include "vk_resources/Image.h"

namespace vk {

	class RenderPass {
		friend class RenderPassBuilder;
	private:
		VkRenderPass render_pass;

	public:
		RenderPass();

	private:
		void destroy();

		inline VkRenderPass handle() { return render_pass; }

		MOVE_SEMANTICS_VK_DEFAULT(RenderPass, render_pass)
	};

	struct AttachmentInfo {
		using Ref = AttachmentInfo&;

		VkFormat format;
		VkAttachmentLoadOp load_op;
		VkAttachmentStoreOp store_op;
		VkImageLayout final_layout;

		AttachmentInfo();

		inline Ref set_format(VkFormat format) { this->format = format; return *this; }
		inline Ref set_load_op(VkAttachmentLoadOp load_op) { this->load_op = load_op; return *this; }
		inline Ref set_store_op(VkAttachmentStoreOp store_op) { this->store_op = store_op; return *this; }
		inline Ref set_final_layout(VkImageLayout final_layout) { this->final_layout = final_layout; return *this; }

		Ref use_swapchain();
	};

	class RenderPassBuilder {
	public:
		using Ref = RenderPassBuilder&;

		AttachmentInfo color_attachment;
		AttachmentInfo depth_attachment;
		bool use_depth_attachment;

		RenderPassBuilder();

		RenderPass build();

		inline Ref set_color_attachment(AttachmentInfo attachment_info) { 
			color_attachment = attachment_info;
			return *this;
		}

		inline Ref set_depth_attachment(AttachmentInfo attachment_info) {
			depth_attachment = attachment_info;
			use_depth_attachment = true;
			return *this;
		}

		inline Ref no_depth_attachment() {
			use_depth_attachment = false;
			return *this;
		}
	};
}