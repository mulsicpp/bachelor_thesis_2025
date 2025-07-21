#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_pipeline/Pipeline.h"
#include "vk_pipeline/Framebuffer.h"

#include "vk_resources/Image.h"

#include <vector>

#include "vk_core/Swapchain.h"

#include "scene/Mesh.h"

class RasterizerBuilder;

class Rasterizer : public utils::Move {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer{};
	ptr::Shared<vk::RenderPass> render_pass{};

	vk::Pipeline pipeline{};

	vk::PassBeginInfo pass_begin_info{};

	vk::CommandBuffer render_cmd_buffer{};

	vk::SubmitInfo submit_info{};

	Mesh rect;

public:
	Rasterizer() = default;

	void draw();
	vk::CommandRecorder draw_triangle_recorder(vk::Framebuffer* framebuffer);

	const ptr::Shared<vk::RenderPass>& get_render_pass() const { return render_pass; }
	const vk::QueueType get_queue_type() const { return vk::QueueType::Graphics; }
};

class RasterizerBuilder {
public:
	using Ref = RasterizerBuilder&;

private:
	vk::Attachment _color_attachment;

public:
	RasterizerBuilder();

	inline Ref color_attachment(const vk::Attachment& color_attachment) { _color_attachment = color_attachment; return *this; }

	Rasterizer build();
};