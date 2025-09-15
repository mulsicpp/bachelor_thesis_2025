#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_pipeline/Pipeline.h"
#include "vk_pipeline/DescriptorPool.h"
#include "vk_pipeline/Framebuffer.h"

#include "scene/Scene.h"

class SkinnerBuilder;

class Skinner : public utils::Move, public ptr::ToShared<Skinner> {
	friend class SkinnerBuilder;
private:
	ptr::Shared<vk::RenderPass> render_pass{};
	ptr::Shared<vk::PipelineLayout> pipeline_layout{};
	vk::Pipeline pipeline{};

	ptr::Shared<vk::Image> image{};
	vk::Framebuffer framebuffer{};

	vk::DescriptorPool descriptor_pool{};

	ptr::Shared<Scene> scene{};

public:
	Skinner() = default;

	void cmd_skin_scene(vk::ReadyCommandBuffer cmd_buf);
	void skin_scene();

	void bind_scene(const ptr::Shared<Scene>& scene);

	inline const vk::QueueType get_queue_type() const { return vk::QueueType::Graphics; }
	inline const ptr::Shared<vk::PipelineLayout>& get_pipeline_layout() const { return pipeline_layout; }
};

class SkinnerBuilder {
public:
	using Ref = SkinnerBuilder&;

public:
	SkinnerBuilder() = default;

	Skinner build() const;
};