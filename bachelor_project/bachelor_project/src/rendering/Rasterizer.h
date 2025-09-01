#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_pipeline/Pipeline.h"
#include "vk_pipeline/Framebuffer.h"
#include "vk_pipeline/DescriptorPool.h"

#include "vk_resources/Image.h"

#include <vector>

#include "vk_core/Swapchain.h"

#include "scene/Scene.h"
#include "scene/Camera.h"


class RasterizerBuilder;

class Rasterizer : public utils::Move, public ptr::ToShared<Rasterizer> {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer{};
	ptr::Shared<vk::RenderPass> render_pass{};

	ptr::Shared<vk::PipelineLayout> pipeline_layout{};
	vk::Pipeline pipeline{};

	vk::PassBeginInfo pass_begin_info{};

	ptr::Shared<vk::Buffer> camera_uniform_buffer;
	ptr::Shared<Scene> scene{};

	vk::DescriptorPool descriptor_pool{};

	Mesh cube;

public:
	Rasterizer() = default;

	inline void bind_camera(const ptr::Shared<Camera>& camera) {
		*camera_uniform_buffer->mapped_data<CameraUBO>() = camera->as_camera_ubo();
		camera_uniform_buffer->flush();
	}

	inline void bind_scene(const ptr::Shared<Scene>& scene) { this->scene = scene; }

	void draw(vk::Framebuffer* framebuffer);
	void cmd_draw(vk::ReadyCommandBuffer cmd_buf, vk::Framebuffer* framebuffer);

	inline const ptr::Shared<vk::RenderPass>& get_render_pass() const { return render_pass; }
	inline const vk::QueueType get_queue_type() const { return vk::QueueType::Graphics; }
	inline const ptr::Shared<vk::PipelineLayout>& get_pipeline_layout() const { return pipeline_layout; }
};

class RasterizerBuilder {
public:
	using Ref = RasterizerBuilder&;

private:
	vk::Attachment _color_attachment;
	vk::Attachment _depth_attachment;

public:
	RasterizerBuilder();

	inline Ref color_attachment(const vk::Attachment& color_attachment) { _color_attachment = color_attachment; return *this; }
	inline Ref depth_attachment(const vk::Attachment& depth_attachment) { _depth_attachment = depth_attachment; return *this; }

	Rasterizer build();
};