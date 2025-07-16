#pragma once

#include "vk_pipeline/RenderPass.h"

#include "vk_resources/Image.h"

class Rasterizer : public utils::Move {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer;
	vk::RenderPass render_pass;

public:
	Rasterizer();
};

class RasterizerBuilder {
public:
	using Ref = RasterizerBuilder&;

	RasterizerBuilder();

	Rasterizer build();
};