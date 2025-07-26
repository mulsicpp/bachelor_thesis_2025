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

	uint32_t max_frames_in_flight{ 1 };
	uint32_t in_flight_index{ 0 };

	std::vector<vk::SubmitInfo> submit_infos{};
	std::vector<vk::CommandBuffer> command_buffers{};

	ptr::Shared<Rasterizer> renderer{};
	std::vector<Frame> frames{};

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

	inline Frame* get_current_frame() { return &frames[in_flight_index]; }

	void draw_frame();

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
	uint32_t _max_frames_in_flight;

public:
	FrameManagerBuilder() = default;

	inline Ref swapchain_builder(const vk::SwapchainBuilder& swapchain_builder) {
		_swapchain_builder = swapchain_builder;
		return *this;
	}

	inline Ref max_frames_in_flight(uint32_t max_frames_in_flight) {
		_max_frames_in_flight = max_frames_in_flight;
		return *this;
	}

	FrameManager build();
};