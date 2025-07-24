#include "Rasterizer.h"

#include "utils/dbg_log.h"

#include "vk_core/Context.h"


void Rasterizer::draw() {

}

vk::CommandRecorder Rasterizer::draw_triangle_recorder(vk::Framebuffer* framebuffer) {
	return [this, framebuffer](vk::ReadyCommandBuffer cmd_buffer) -> void {
		framebuffer->cmd_begin_pass(cmd_buffer, pass_begin_info);

		pipeline.cmd_bind(cmd_buffer);

		VkBuffer vertex_buffer = rect.vertex_buffer.handle();
		VkDeviceSize offset = 0;

		vkCmdBindVertexBuffers(cmd_buffer.handle(), 0, 1, &vertex_buffer, &offset);
		vkCmdBindIndexBuffer(cmd_buffer.handle(), rect.index_buffer.handle(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmd_buffer.handle(), 6, 1, 0, 0, 0);

		framebuffer->cmd_end_pass(cmd_buffer);
		};
}


RasterizerBuilder::RasterizerBuilder()
{}

Rasterizer RasterizerBuilder::build() {
	Rasterizer rasterizer;

	rasterizer.render_pass = vk::RenderPassBuilder()
		.add_attachment(_color_attachment)
		.build()
		.to_shared();

	dbg_log("created render pass");

	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex()
		.load_spirv("assets/shaders/mesh2d/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment()
		.load_spirv("assets/shaders/mesh2d/frag.spv")
		.build();
	dbg_log("loaded fragment shader");

	rasterizer.pipeline_layout = vk::PipelineLayoutBuilder()
		.add_layout(vk::DescriptorSetLayoutBuilder()
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.build())
		.add_layout(vk::DescriptorSetLayoutBuilder()
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.build())
		.build()
		.to_shared();

	rasterizer.pipeline = vk::PipelineBuilder()
		.render_pass(rasterizer.render_pass)
		.add_shader(std::move(vertex_shader))
		.add_shader(std::move(fragment_shader))
		.layout(rasterizer.pipeline_layout)
		.vertex_input(Mesh::get_vertex_input())
		.build();

	dbg_log("created pipeline");

	rasterizer.pass_begin_info = vk::PassBeginInfo()
		.add_clear_value(vk::ClearValue::color(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }));

	rasterizer.rect = Mesh::create_rect();

	return rasterizer;
}