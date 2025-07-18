#pragma once

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_pipeline/Pipeline.h"

#include "vk_resources/Image.h"


class RasterizerBuilder;

class Rasterizer : public utils::Move {
	friend class RasterizerBuilder;
private:
	vk::Image depth_buffer{};
	ptr::Shared<vk::RenderPass> render_pass{};

	vk::Pipeline pipeline{};

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