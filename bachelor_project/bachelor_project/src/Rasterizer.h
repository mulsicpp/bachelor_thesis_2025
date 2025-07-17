#pragma once

#include "utils/ptr_alias.h"

#include "vk_pipeline/Pipeline.h"

#include "vk_resources/Image.h"

class Rasterizer : public utils::Move {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer;
	ptr::Shared<vk::RenderPass> render_pass;

	vk::Pipeline pipeline;

public:
	Rasterizer();
};

class RasterizerBuilder {
public:
	using Ref = RasterizerBuilder&;

	RasterizerBuilder();

	Rasterizer build();
};