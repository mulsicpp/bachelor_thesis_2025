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
		.set_color_attachment(vk::AttachmentInfo().use_swapchain())
		.no_depth_attachment()
		.build();

	dbg_log("created render pass");

	return rasterizer;
}