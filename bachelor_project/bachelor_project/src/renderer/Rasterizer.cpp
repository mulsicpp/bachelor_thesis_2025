#include "Rasterizer.h"

#include "utils/dbg_log.h"



void Rasterizer::draw() {

}



RasterizerBuilder::RasterizerBuilder()
{}

Rasterizer RasterizerBuilder::build() {
	Rasterizer rasterizer;

	rasterizer.render_pass = vk::RenderPassBuilder()
		.add_attachment(vk::Attachment().color().from_swapchain())
		.build().to_shared();

	dbg_log("created render pass");
	
	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex()
		.load_spirv("assets/shaders/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment()
		.load_spirv("assets/shaders/frag.spv")
		.build();
	dbg_log("loaded fragment shader");

	rasterizer.pipeline = vk::PipelineBuilder()
		.render_pass(rasterizer.render_pass)
		.add_shader(std::move(vertex_shader))
		.add_shader(std::move(fragment_shader))
		.build();

	dbg_log("created pipeline");

	return rasterizer;
}