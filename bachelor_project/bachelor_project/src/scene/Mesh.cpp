#include "Mesh.h"

#include "vk_core/format.h"

static const std::vector<Vertex> rect_vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}},
	{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<uint16_t> rect_indices = {
	0, 1, 2, 0, 2, 3
};

vk::VertexInput Mesh::get_vertex_input() {
	return vk::VertexInput()
		.add_binding_info(vk::VertexBindingInfo().set_stride(sizeof(Vertex)))

		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(0)
			//.set_format(vk::format_of_type<decltype(Vertex::position)>())
			.set_format(VK_FORMAT_R32G32_SFLOAT)
			.set_offset(offsetof(Vertex, position)))

		.add_attribute_info(vk::VertexAttributeInfo()
			.set_binding(0)
			//.set_format(vk::format_of_type<decltype(Vertex::color)>())
			.set_format(VK_FORMAT_R32G32B32_SFLOAT)
			.set_offset(offsetof(Vertex, color)));
}

Mesh Mesh::create_rect() {
	Mesh mesh;

	mesh.vertex_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(rect_vertices.size() * sizeof(Vertex))
		.data((void*)rect_vertices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build();

	mesh.index_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.size(rect_indices.size() * sizeof(uint16_t))
		.data((void*)rect_indices.data())
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.build();

	return mesh;
}