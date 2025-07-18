#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

#include "vk_resources/Image.h"

namespace vk {

	class RenderPassBuilder;

	class RenderPass : public utils::Move, public ptr::ToShared<RenderPass> {
		friend class RenderPassBuilder;
	private:
		Handle<VkRenderPass> render_pass{};

	public:
		RenderPass() = default;

		inline VkRenderPass handle() const { return *render_pass; }
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

		Ref from_swapchain();
	};

	class RenderPassBuilder {
	public:
		using Ref = RenderPassBuilder&;

	private:
		AttachmentInfo _color_attachment;
		AttachmentInfo _depth_attachment;
		bool _use_depth_attachment;

	public:
		RenderPassBuilder();


		inline Ref color_attachment(AttachmentInfo attachment_info) { 
			_color_attachment = attachment_info;
			return *this;
		}

		inline Ref depth_attachment(AttachmentInfo attachment_info) {
			_depth_attachment = attachment_info;
			_use_depth_attachment = true;
			return *this;
		}


		RenderPass build();
	};
}