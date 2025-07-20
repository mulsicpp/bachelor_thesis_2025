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

	struct ClearValue {
	private:
		VkClearValue clear_value;

		inline ClearValue(const VkClearValue& value) : clear_value{ value } {}

	public:
		static ClearValue color(std::array<float, 4> color);
		static ClearValue color(std::array<int32_t, 4> color);
		static ClearValue color(std::array<uint32_t, 4> color);

		static ClearValue depth(float depth);

		inline const VkClearValue& get() const { return clear_value; }
	};

	struct PassBeginInfo {
		using Ref = PassBeginInfo&;
		VkRect2D render_area{ { 0,0 }, { UINT32_MAX, UINT32_MAX } };
		std::vector<ClearValue> clear_values{};

		PassBeginInfo() = default;

		inline Ref set_render_area(VkRect2D render_area) { this->render_area = render_area; return *this; }

		inline Ref set_clear_values(const std::vector<ClearValue>& clear_values) { this->clear_values = clear_values; return *this; }
		inline Ref add_clear_value(const ClearValue& clear_value) { this->clear_values.push_back(clear_value); return *this; }
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
		inline Ref set_type(AttachmentType type) { this->type = type; return *this; }

		inline Ref color() { type = AttachmentType::Color; return *this; }
		inline Ref depth() { type = AttachmentType::Depth; return *this; }

		Ref from_swapchain(const Swapchain* swapchain);
	};

	class RenderPassBuilder;
	class Framebuffer;

	class RenderPass : public utils::Move, public ptr::ToShared<RenderPass> {
		friend class RenderPassBuilder;
		friend class Framebuffer;
	private:
		Handle<VkRenderPass> render_pass{};

		std::vector<Attachment> _attachments{};

		bool _active{ false };
		PassBeginInfo _begin_info;

	public:
		RenderPass() = default;

		inline VkRenderPass handle() const { return *render_pass; }
		inline const std::vector<Attachment>& attachments() const { return _attachments; }

		inline bool active() const { return _active; }
		inline const PassBeginInfo& begin_info() const { return _begin_info; }
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