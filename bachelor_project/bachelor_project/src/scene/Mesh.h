#pragma once

#include <glm/glm.hpp>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_resources/Buffer.h"
#include "vk_pipeline/VertexInput.h"

struct Vertex {
	glm::vec3 position;
};

struct Mesh : public utils::Move, public ptr::ToShared<Mesh> {
	vk::Buffer vertex_buffer{};
	vk::Buffer index_buffer{};

	static vk::VertexInput get_vertex_input();
	static Mesh create_cube();
};