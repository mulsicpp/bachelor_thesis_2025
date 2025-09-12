#include "Skinner.h"

#include "utils/dbg_log.h"
#include "utils/AppPath.h"

void Skinner::cmd_skin_scene(vk::ReadyCommandBuffer cmd_buf)
{
	framebuffer.cmd_begin_pass(cmd_buf, {});

	pipeline.cmd_bind(cmd_buf);

	descriptor_pool.cmd_bind_set(cmd_buf, 0, VK_PIPELINE_BIND_POINT_GRAPHICS);

	auto iterator = scene->iter();
	while (iterator.has_next())
	{
		const auto& node = iterator.next();

		const auto& mesh = node->mesh;
		if (!mesh || !node->skin)
		{
			continue;
		}

		const auto& joint_matrices_buffer = node->joint_matrices.buffer();
		glm::mat4* joint_matrices = (glm::mat4*)(joint_matrices_buffer->mapped_data() + node->joint_matrices.offset());

		for (uint32_t i = 0; i < node->skin->nodes.size(); i++) {
			joint_matrices[i] = glm::inverse(node->global_transform) * node->skin->nodes[i]->global_transform * node->skin->inverse_bind_matrices[i];
		}
		scene->get_buffers().joint_matrices->flush();

		for (uint32_t i = 0; i < mesh->primitives.size(); i++) {
			mesh->primitives[i].skin(cmd_buf, &pipeline, node->dynamic_positions[i], node->joint_matrices);
		}
	}

	framebuffer.cmd_end_pass(cmd_buf);
}

void Skinner::skin_scene()
{
	vk::CommandBuffer::single_time_submit(vk::QueueType::Graphics, [&](vk::ReadyCommandBuffer cmd_buffer)
		{ this->cmd_skin_scene(cmd_buffer); });
}

void Skinner::bind_scene(const ptr::Shared<Scene>& scene) {
	this->scene = scene;

	const auto& buffers = scene->get_buffers();
	descriptor_pool.update_set_binding(0, 0, vk::BufferDescriptorInfo(buffers.dynamic_positions));
	descriptor_pool.update_set_binding(0, 1, vk::BufferDescriptorInfo(buffers.joint_matrices));
	descriptor_pool.update_set_binding(0, 2, vk::BufferDescriptorInfo(buffers.joint_weights));
}

Skinner SkinnerBuilder::build() const {
	Skinner skinner{};

	skinner.image = vk::ImageBuilder()
        .extent({ 1, 1 })
        .format(VK_FORMAT_R8G8B8A8_UNORM)
        .queue_types({ vk::QueueType::Transfer, vk::QueueType::Graphics })
        .usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .aspect(VK_IMAGE_ASPECT_COLOR_BIT)
        .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
        .tiling(VK_IMAGE_TILING_OPTIMAL)
        .build().to_shared();

	auto attachment = vk::Attachment()
        .set_type(vk::AttachmentType::Color)
        .set_format(skinner.image->format())
        .set_load_op(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
        .set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE)
        .set_final_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	const auto& app_path = utils::AppPath::instance();

	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex_stage()
		.load_spirv(app_path.get_path("../../assets/shaders/skinning/vert.spv"))
		.build();
	dbg_log("loaded vertex shader");

	skinner.pipeline_layout = vk::PipelineLayoutBuilder()
		.add_layout(vk::DescriptorSetLayoutBuilder()
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.add_binding(vk::DescriptorSetLayoutBinding()
				.set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
				.set_stage_flags(VK_SHADER_STAGE_VERTEX_BIT))
			.build())
		.push_constant(vk::PushConstant()
			.add_stage_flag(VK_SHADER_STAGE_VERTEX_BIT)
			.set_size(sizeof(SkinningPushConstant)))
		.build()
		.to_shared();

	skinner.render_pass = vk::RenderPassBuilder()
		.add_attachment(attachment)
		.build().to_shared();

	skinner.pipeline = vk::PipelineBuilder()
		.render_pass(skinner.render_pass)
		.add_shader(std::move(vertex_shader))
		.layout(skinner.pipeline_layout)
		.vertex_input(Primitive::get_vertex_input())
		.rasterizer_discard(true)
		.topology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
		.build();

	skinner.framebuffer = vk::FramebufferBuilder()
        .render_pass(skinner.render_pass)
        .add_image(skinner.image)
        .build();

	skinner.descriptor_pool = vk::DescriptorPoolBuilder()
		.pipeline_layout(skinner.pipeline_layout)
		.build();

	return skinner;
}