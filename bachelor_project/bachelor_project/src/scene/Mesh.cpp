#include "Mesh.h"

#include "vk_core/format.h"

static const std::vector<Vertex> cube_vertices = {
	{{-0.5f, -0.5f, +0.5f}},
	{{+0.5f, -0.5f, +0.5f}},
	{{+0.5f, +0.5f, +0.5f}},
	{{-0.5f, +0.5f, +0.5f}},
	{{-0.5f, -0.5f, -0.5f}},
	{{+0.5f, -0.5f, -0.5f}},
	{{+0.5f, +0.5f, -0.5f}},
	{{-0.5f, +0.5f, -0.5f}},
};

const std::vector<uint16_t> cube_indices = {
	0, 1, 2, 0, 2, 3,
	0, 4, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 4,
	6, 5, 4, 6, 4, 7,
	6, 7, 3, 6, 3, 2,
	6, 2, 1, 6, 1, 5,
};

vk::VertexInput Mesh::get_vertex_input() {
	return vk::VertexInput()
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(Vertex)))

		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(0)
			.set_format(vk::format_of_type<decltype(Vertex::position)>())
			.set_offset(offsetof(Vertex, position)));
}

Mesh Mesh::create_cube() {
	Mesh mesh;

	mesh.vertex_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(static_cast<uint32_t>(cube_vertices.size() * sizeof(Vertex)))
		.data((void*)cube_vertices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build();

	mesh.index_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(static_cast<uint32_t>(cube_indices.size() * sizeof(uint16_t)))
		.data((void*)cube_indices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build();

	return mesh;
}