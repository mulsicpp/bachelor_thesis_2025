#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_pipeline/Pipeline.h"
#include "vk_pipeline/Framebuffer.h"

#include "vk_resources/Image.h"

#include <vector>


class RasterizerBuilder;

class Rasterizer : public utils::Move {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer{};
	ptr::Shared<vk::RenderPass> render_pass{};

	vk::Pipeline pipeline{};

	std::vector<vk::Framebuffer> framebuffers{};

	vk::CommandBuffer render_cmd_buffer;

public:
	Rasterizer() = default;

	void draw();
};

class RasterizerBuilder {
public:
	using Ref = RasterizerBuilder&;

	RasterizerBuilder();

	Rasterizer build();
};