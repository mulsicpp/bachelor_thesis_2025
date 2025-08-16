#include "Mesh.h"

#include "vk_core/format.h"


const ptr::Shared<Material> Material::default_material = ptr::make_shared<Material>(Material{});

void Primitive::draw(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, const glm::mat4& global_transform) const {
	MeshPushConst mesh_push_const{};

	mesh_push_const.transform = global_transform;
	mesh_push_const.base_color = material->base_color;
	pipeline->cmd_push_constant(cmd_buffer, &mesh_push_const);

	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 0, positions.buffer().get(), positions.offset());

	if (indices.buffer()) {
		vk::Pipeline::cmd_bind_index_buffer(cmd_buffer, indices.buffer().get(), Primitive::get_index_type(), indices.offset());
		vk::Pipeline::cmd_draw_indexed(cmd_buffer, get_index_count(), 1);
	}
	else {
		vk::Pipeline::cmd_draw(cmd_buffer, positions.length() / sizeof(PositionType), 1);
	}
}

void Primitive::draw_dynamic(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, const glm::mat4& global_transform, const vk::SubBuffer& dynamic_positions) const {
	MeshPushConst mesh_push_const{};

	mesh_push_const.transform = global_transform;
	mesh_push_const.base_color = material->base_color;
	pipeline->cmd_push_constant(cmd_buffer, &mesh_push_const);

	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 0, dynamic_positions.buffer().get(), dynamic_positions.offset());

	if (indices.buffer()) {
		vk::Pipeline::cmd_bind_index_buffer(cmd_buffer, indices.buffer().get(), Primitive::get_index_type(), indices.offset());
		vk::Pipeline::cmd_draw_indexed(cmd_buffer, get_index_count(), 1);
	}
	else {
		vk::Pipeline::cmd_draw(cmd_buffer, dynamic_positions.length() / sizeof(PositionType), 1);
	}
}



static const std::vector<Primitive::PositionType> cube_vertices = {
	{-0.5f, -0.5f, +0.5f},
	{+0.5f, -0.5f, +0.5f},
	{+0.5f, +0.5f, +0.5f},
	{-0.5f, +0.5f, +0.5f},
	{-0.5f, -0.5f, -0.5f},
	{+0.5f, -0.5f, -0.5f},
	{+0.5f, +0.5f, -0.5f},
	{-0.5f, +0.5f, -0.5f},
};

const std::vector<Primitive::IndexType> cube_indices = {
	0, 1, 2, 0, 2, 3,
	0, 4, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 4,
	6, 5, 4, 6, 4, 7,
	6, 7, 3, 6, 3, 2,
	6, 2, 1, 6, 1, 5,
};

vk::VertexInput Primitive::get_vertex_input() {
	return vk::VertexInput()
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(PositionType)))
		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(0)
			.set_format(vk::format_of_type<PositionType>()));
}

Mesh Mesh::create_cube() {
	Mesh mesh;

	Primitive primitive{};

	primitive.positions = vk::SubBuffer::from(
		vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(cube_vertices.size() * sizeof(Primitive::PositionType))
		.data((void*)cube_vertices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build()
	);

	primitive.indices = vk::SubBuffer::from(
		vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(cube_indices.size() * sizeof(Primitive::IndexType))
		.data((void*)cube_indices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build()
	);

	primitive.topology = Primitive::Topology::Triangles;
	primitive.material = Material::default_material;

	mesh.primitives.push_back(primitive);

	return mesh;
}