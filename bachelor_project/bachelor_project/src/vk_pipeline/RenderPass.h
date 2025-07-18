#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"
#include "vk_core/CommandBuffer.h"

#include "vk_resources/Image.h"

#include <array>
#include <vector>

namespace vk {

	class RenderPassBuilder;
	struct Attachment;

	class RenderPass : public utils::Move, public ptr::ToShared<RenderPass> {
		friend class RenderPassBuilder;
	private:
		Handle<VkRenderPass> render_pass{};

		std::vector<Attachment> _attachments{};

	public:
		RenderPass() = default;

		inline VkRenderPass handle() const { return *render_pass; }
		inline const std::vector<Attachment>& attachments() const { return _attachments; }
	};

	enum class AttachmentType {
		Color,
		Depth
	};

	struct Attachment {
		using Ref = Attachment&;

		AttachmentType type;
		VkFormat format;
		VkAttachmentLoadOp load_op;
		VkAttachmentStoreOp store_op;
		VkImageLayout final_layout;

		Attachment();

		inline Ref set_format(VkFormat format) { this->format = format; return *this; }
		inline Ref set_load_op(VkAttachmentLoadOp load_op) { this->load_op = load_op; return *this; }
		inline Ref set_store_op(VkAttachmentStoreOp store_op) { this->store_op = store_op; return *this; }
		inline Ref set_final_layout(VkImageLayout final_layout) { this->final_layout = final_layout; return *this; }

		inline Ref color() { type = AttachmentType::Color; return *this; }
		inline Ref depth() { type = AttachmentType::Depth; return *this; }

		Ref from_swapchain();
	};

	class RenderPassBuilder {
	public:
		using Ref = RenderPassBuilder&;

	private:
		std::vector<Attachment> _attachments;

	public:
		RenderPassBuilder();


		inline Ref attachments(const std::vector<Attachment>& attachments) { 
			_attachments = attachments;
			return *this;
		}

		inline Ref add_attachment(Attachment attachment) {
			_attachments.push_back(attachment);
			return *this;
		}


		RenderPass build();
	};
}