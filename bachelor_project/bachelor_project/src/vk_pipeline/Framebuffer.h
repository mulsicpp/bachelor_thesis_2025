#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"
#include "vk_core/CommandBuffer.h"

#include "vk_resources/ImageView.h"
#include "RenderPass.h"

#include <vector>
#include <array>


namespace vk {

	class FramebufferBuilder;

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
		VkRect2D render_area;
		std::vector<ClearValue> clear_values;
	};

	class Framebuffer : public utils::Move, public ptr::ToShared<Framebuffer> {
		friend class FramebufferBuilder;
	private:
		ptr::Shared<const RenderPass> _render_pass{};
		std::vector<ImageView> _image_views{};
		Handle<VkFramebuffer> framebuffer{};

		VkExtent2D _extent;

	public:
		Framebuffer() = default;

		inline VkFramebuffer handle() const { return *framebuffer; }

		inline const ptr::Shared<const RenderPass>& render_pass() const { return _render_pass; }
		inline const std::vector<ImageView>& image_views() const { return _image_views; }
		inline const VkExtent2D& extent() const { return _extent; }

		void cmd_begin_pass(ReadyCommandBuffer cmd_buffer, const PassBeginInfo& info);
		void cmd_end_pass(ReadyCommandBuffer cmd_buffer);
	};

	class FramebufferBuilder {
	public:
		using Ref = FramebufferBuilder&;

	private:
		ptr::Shared<const RenderPass> _render_pass{};
		std::vector<ptr::Shared<const Image>> _images{};

	public:
		FramebufferBuilder() = default;

		Ref render_pass(RenderPass&& render_pass) { _render_pass = std::move(render_pass).to_shared(); return *this; }
		Ref render_pass(const ptr::Shared<RenderPass>& render_pass) { _render_pass = render_pass; return *this; }

		Ref images(const std::vector<ptr::Shared<const Image>>& images) { _images = images; return *this; }
		Ref add_image(Image&& image) { _images.push_back(std::move(image).to_shared()); return *this; }
		Ref add_image(const ptr::Shared<const Image>& image) { _images.push_back(image); return *this; }

		Framebuffer build();
	};
}