#include "Rasterizer.h"

#include "utils/dbg_log.h"

Rasterizer::Rasterizer()
	: depth_buffer{}
	, render_pass{}
{}

RasterizerBuilder::RasterizerBuilder()
{}

Rasterizer RasterizerBuilder::build() {
	Rasterizer rasterizer;

	rasterizer.render_pass = vk::RenderPassBuilder()
		.color_attachment(vk::AttachmentInfo().from_swapchain())
		.build();

	dbg_log("created render pass");

	return rasterizer;
}