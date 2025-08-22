#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Swapchain.h"
#include "vk_core/CommandBuffer.h"

#include "vk_pipeline/Framebuffer.h"
#include "vk_pipeline/DescriptorPool.h"

#include <vector>

#include "Frame.h"
#include "Rasterizer.h"

typedef vk::CommandRecorder(Rasterizer::*DrawRecorder)(vk::Framebuffer* framebuffer);

class FrameManagerBuilder;

class FrameManager : public utils::Move, public ptr::ToShared<FrameManager> {
	friend class FrameManagerBuilder;
private:
	vk::Swapchain swapchain{};

	vk::SubmitInfo submit_info{};
	vk::CommandBuffer command_buffer{};

	ptr::Shared<Rasterizer> renderer{};

	ptr::Shared<vk::Image> depth_image{};
	std::vector<vk::Framebuffer> framebuffers{};

	bool resize_signaled{ false };

public:
	FrameManager() = default;

	inline VkExtent2D get_framebuffer_extent() const { return swapchain.extent(); }

	vk::Attachment get_swapchain_attachment() const;
	vk::Attachment get_depth_attachment() const;

	void bind_rasterizer(Rasterizer&& rasterizer);
	void bind_rasterizer(const ptr::Shared<Rasterizer>& rasterizer);

	void draw_frame(const Frame& frame);

	inline void signal_resize() { resize_signaled = true; }

private:
	void create_framebuffers();
	void recreate();
};

class FrameManagerBuilder {
public:
	using Ref = FrameManagerBuilder&;

private:
	vk::SwapchainBuilder _swapchain_builder{};

public:
	FrameManagerBuilder() = default;

	inline Ref swapchain_builder(const vk::SwapchainBuilder& swapchain_builder) {
		_swapchain_builder = swapchain_builder;
		return *this;
	}

	FrameManager build();
};