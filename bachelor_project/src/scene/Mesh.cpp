#include "Mesh.h"

#include "vk_core/format.h"


const ptr::Shared<Material> Material::default_material = ptr::make_shared<Material>(Material{});

void Primitive::draw(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, const glm::mat4& global_transform, vk::SubBuffer dynamic_positions) const {
	MeshPushConst mesh_push_const{};

	mesh_push_const.transform = global_transform;
	mesh_push_const.base_color = material->base_color;
	pipeline->cmd_push_constant(cmd_buffer, &mesh_push_const);

	const vk::SubBuffer& draw_positions = dynamic_positions.buffer() ? dynamic_positions : positions;

	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 0, draw_positions.buffer().get(), draw_positions.offset());

	if (indices.buffer()) {
		vk::Pipeline::cmd_bind_index_buffer(cmd_buffer, indices.buffer().get(), Primitive::get_index_type(), indices.offset());
		vk::Pipeline::cmd_draw_indexed(cmd_buffer, get_index_count(), 1);
	}
	else {
		vk::Pipeline::cmd_draw(cmd_buffer, draw_positions.length() / sizeof(PositionType), 1);
	}
}

void Primitive::skin(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, vk::SubBuffer dynamic_positions, vk::SubBuffer joint_matrices, vk::SubBuffer dynamic_normals, vk::SubBuffer dynamic_tangents) const {
	SkinningPushConstant skinning_push_const{};

	vk::SubBuffer binding_normals{};
	vk::SubBuffer binding_tangents{};

	skinning_push_const.dynamic_positions_offset = dynamic_positions.offset() / sizeof(PositionType);

	if(dynamic_normals.buffer()) {
		skinning_push_const.dynamic_normals_offset = dynamic_normals.offset() / sizeof(PositionType);
		binding_normals = normals;
	} else {
		skinning_push_const.dynamic_normals_offset = ~uint32_t(0);
		binding_normals = positions;
	}

	if(dynamic_tangents.buffer()) {
		skinning_push_const.dynamic_tangents_offset = dynamic_tangents.offset() / sizeof(PositionType);
		binding_tangents = tangents;
	} else {
		skinning_push_const.dynamic_tangents_offset = ~uint32_t(0);
		binding_tangents = positions;
	}

	skinning_push_const.joint_matrices_offset = joint_matrices.offset() / sizeof(glm::mat4);
	skinning_push_const.joint_weights_offset = joint_weights.offset() / sizeof(JointWeight);
	skinning_push_const.joint_weights_per_vertex = joint_weights_cpu.size() > 0 ? joint_weights_cpu[0].size() : 0;

	pipeline->cmd_push_constant(cmd_buffer, &skinning_push_const);

	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 0, positions.buffer().get(), positions.offset());
	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 1, binding_normals.buffer().get(), binding_normals.offset());
	vk::Pipeline::cmd_bind_vertex_buffer(cmd_buffer, 2, binding_tangents.buffer().get(), binding_tangents.offset());
	
	vk::Pipeline::cmd_draw(cmd_buffer, positions.length() / sizeof(PositionType), 1);
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

vk::VertexInput Primitive::get_skinning_vertex_input() {
	return vk::VertexInput()
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(PositionType)))
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(PositionType)))
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(PositionType)))
		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(0)
			.set_format(vk::format_of_type<PositionType>()))
		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(1)
			.set_format(vk::format_of_type<PositionType>()))
		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(2)
			.set_format(vk::format_of_type<PositionType>()));
}

vk::BlasGeometry Primitive::get_blas_geometry(const vk::SubBuffer& dynamic_positions) const {
	vk::BlasGeometry geometry{};

	const vk::SubBuffer& blas_positions = dynamic_positions.buffer() ? dynamic_positions : positions;

	geometry.set_position_input(blas_positions, blas_positions.length() / sizeof(PositionType), get_vertex_input(), 0);

	if (indices.buffer()) {
		geometry.set_index_input(indices, get_index_type());
		geometry.set_triangle_count(get_index_count() / 3);
	}
	else {
		geometry.set_triangle_count(blas_positions.length() / (sizeof(PositionType) * 3));
	}

	geometry.set_opaque((material->flags & MAT_ALPHA_MODE) == ALPHA_MODE_OPAQUE);

	return geometry;
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
	primitive.material = Material::default_material.get();

	mesh.primitives.push_back(primitive);

	return mesh;
}

std::vector<vk::BlasGeometry> Mesh::get_blas_geometries() const {
	std::vector<vk::BlasGeometry> geometries{};
	geometries.reserve(primitives.size());

	for (const auto& primitive : primitives) {
		geometries.push_back(primitive.get_blas_geometry());
	}

	return geometries;
}

void Mesh::build_blas() {
	blas = vk::BlasBuilder()
		.geometries(get_blas_geometries())
		.dynamic(false)
		.build().to_shared();
}