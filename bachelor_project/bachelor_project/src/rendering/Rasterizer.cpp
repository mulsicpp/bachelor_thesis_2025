#include "Rasterizer.h"

#include "external/glm.h"

#include "utils/dbg_log.h"

#include "vk_core/Context.h"


void Rasterizer::cmd_draw_frame(vk::ReadyCommandBuffer cmd_buf, Frame* frame, vk::Framebuffer* framebuffer) {
	std::vector<glm::mat4> transforms{};
	std::vector<ptr::Shared<Mesh>> meshes{};

	auto iterator = frame->scene->iter();

	while (iterator.has_next()) {
		const auto& node = iterator.next();
		transforms.push_back(node->global_transform);
		meshes.push_back(node->mesh);
	}

	if (frame->model_count < transforms.size()) {
		frame->model_uniform_buffer = vk::BufferBuilder()
			.usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
			.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
			.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
			.size(sizeof(ModelUBO) * transforms.size())
			.build().to_shared();
		frame->p_model_ubo = frame->model_uniform_buffer->mapped_data<ModelUBO>();
		frame->model_count = transforms.size();

		frame->descriptor_pool.update_set_binding(1, 0, { frame->model_uniform_buffer, 0, sizeof(ModelUBO) });

		dbg_log("descriptor set binding was updated!");
	}

	glm::mat4 scale = glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 0.03f });

	for (uint32_t i = 0; i < transforms.size(); i++) {
		frame->p_model_ubo[i].transform = transforms[i];
	}


	framebuffer->cmd_begin_pass(cmd_buf, pass_begin_info);

	pipeline.cmd_bind(cmd_buf);

	frame->descriptor_pool.cmd_bind_set(cmd_buf, 0);



	std::vector<uint32_t> offsets{};
	offsets.resize(1);
	for (uint32_t i = 0; i < transforms.size(); i++) {
		offsets[0] = sizeof(ModelUBO) * i;
		frame->descriptor_pool.cmd_bind_set(cmd_buf, 1, offsets);

		// vk::Pipeline::cmd_bind_vertex_buffer(cmd_buf, 0, cube.primitives[0].positions.buffer().get());
		// vk::Pipeline::cmd_bind_index_buffer(cmd_buf, cube.primitives[0].indices.buffer().get(), Primitive::get_index_type());

		// vk::Pipeline::cmd_draw_indexed(cmd_buf, 36, 1);

		if (!meshes[i]) {
			continue;
		}

		for (const auto& primitive : meshes[i]->primitives) {
			vk::Pipeline::cmd_bind_vertex_buffer(cmd_buf, 0, primitive.positions.buffer().get(), primitive.positions.offset());
			vk::Pipeline::cmd_bind_index_buffer(cmd_buf, primitive.indices.buffer().get(), Primitive::get_index_type(), primitive.indices.offset());

			vk::Pipeline::cmd_draw_indexed(cmd_buf, primitive.get_index_count(), 1);
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

	frame.model_uniform_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		.queue_types({ vk::QueueType::Graphics })
		.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
		.size(sizeof(ModelUBO))
		.build().to_shared();
	frame.p_model_ubo = frame.model_uniform_buffer->mapped_data<ModelUBO>();
	frame.model_count = 1;

	frame.descriptor_pool = vk::DescriptorPoolBuilder()
		.pipeline_layout(pipeline_layout)
		.add_set(vk::DescriptorSetInfo()
			.set_index(0)
			.set_binding(0, frame.camera_uniform_buffer))
		.add_set(vk::DescriptorSetInfo()
			.set_index(1)
			.set_binding(0, { frame.model_uniform_buffer, 0, sizeof(ModelUBO) }))
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
		.vertex()
		.load_spirv("assets/shaders/mesh3d/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment()
		.load_spirv("assets/shaders/mesh3d/frag.spv")
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
				.set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.build())
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