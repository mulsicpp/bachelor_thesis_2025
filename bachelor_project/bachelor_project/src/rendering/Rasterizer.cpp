#include "Rasterizer.h"

#include "external/glm.h"

#include "utils/dbg_log.h"

#include "vk_core/Context.h"


void Rasterizer::cmd_draw_frame(vk::ReadyCommandBuffer cmd_buf, const Frame& frame, vk::Framebuffer* framebuffer) {
	std::vector<glm::mat4> transforms{};
	std::vector<ptr::Shared<Mesh>> meshes{};


	framebuffer->cmd_begin_pass(cmd_buf, pass_begin_info);

	pipeline.cmd_bind(cmd_buf);

	frame.descriptor_pool.cmd_bind_set(cmd_buf, 0);

	auto iterator = frame.scene->iter();
	while (iterator.has_next()) {
		const auto& node = iterator.next();

		cube.primitives[0].draw(cmd_buf, &pipeline, node->global_transform * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 0.04f }));

		const auto& mesh = node->mesh;
		if (!mesh) {
			continue;
		}

		for (uint32_t i = 0; i < mesh->primitives.size(); i++) {
			mesh->primitives[i].draw_dynamic(cmd_buf, &pipeline, node->global_transform, node->dynamic_positions[i]);
		}
	}

	framebuffer->cmd_end_pass(cmd_buf);
}



Frame Rasterizer::create_frame() const {
	Frame frame;

	frame.camera_uniform_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		.queue_types({ vk::QueueType::Graphics })
		.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
		.size(sizeof(CameraUBO))
		.build().to_shared();
	frame.p_camera_ubo = frame.camera_uniform_buffer->mapped_data<CameraUBO>();

	frame.descriptor_pool = vk::DescriptorPoolBuilder()
		.pipeline_layout(pipeline_layout)
		.add_set(vk::DescriptorSetInfo()
			.set_index(0)
			.set_binding(0, frame.camera_uniform_buffer))
		.build();

	return frame;
}


std::vector<Frame> Rasterizer::create_frames(uint32_t count) const {
	std::vector<Frame> frames{ count };

	for (uint32_t i = 0; i < count; i++) {
		frames[i] = create_frame();
	}

	return frames;
}




RasterizerBuilder::RasterizerBuilder()
{}

Rasterizer RasterizerBuilder::build() {
	Rasterizer rasterizer;

	rasterizer.render_pass = vk::RenderPassBuilder()
		.add_attachment(_color_attachment)
		.add_attachment(_depth_attachment)
		.build()
		.to_shared();

	dbg_log("created render pass");

	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex_stage()
		.load_spirv("assets/shaders/mesh3d/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment_stage()
		.load_spirv("assets/shaders/mesh3d/frag.spv")
		.build();
	dbg_log("loaded fragment shader");

	rasterizer.pipeline_layout = vk::PipelineLayoutBuilder()
		.add_layout(vk::DescriptorSetLayoutBuilder()
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.build())
		.push_constant(vk::PushConstant()
			.add_stage_flag(VK_SHADER_STAGE_VERTEX_BIT)
			.set_size(sizeof(MeshPushConst)))
		.build()
		.to_shared();

	rasterizer.pipeline = vk::PipelineBuilder()
		.render_pass(rasterizer.render_pass)
		.add_shader(std::move(vertex_shader))
		.add_shader(std::move(fragment_shader))
		.layout(rasterizer.pipeline_layout)
		.vertex_input(Primitive::get_vertex_input())
		.build();

	dbg_log("created pipeline");

	rasterizer.pass_begin_info = vk::PassBeginInfo()
		.add_clear_value(vk::ClearValue::color(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f }))
		.add_clear_value(vk::ClearValue::depth(1.0f));

	rasterizer.cube = Mesh::create_cube();

	return rasterizer;
}