#pragma once

#include <glm/glm.hpp>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/CommandBuffer.h"
#include "vk_resources/SubBuffer.h"

#include "vk_pipeline/Pipeline.h"

struct Material {
	static const ptr::Shared<Material> default_material;

	glm::vec4 base_color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct Primitive {
	using PositionType = glm::vec3;
	using UVType = glm::vec2;
	using ColorType = glm::vec<3, uint8_t>;

	using IndexType = uint32_t;

	vk::SubBuffer positions{};
	vk::SubBuffer uvs{};
	vk::SubBuffer colors{};

	vk::SubBuffer indices{};

	enum class Topology {
		Triangles,
		TriangleStrip,
		TriangleFan,
	} topology{ Topology::Triangles };

	ptr::Shared<Material> material{};

	void draw(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, const glm::mat4& global_transform) const;

	static vk::VertexInput get_vertex_input();

	inline static constexpr VkIndexType get_index_type() {
		return sizeof(IndexType) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	}

	inline VkPrimitiveTopology get_vk_topology() const {
		switch (topology) {
		case Topology::Triangles:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case Topology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case Topology::TriangleFan:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		}
	}

	inline VkDeviceSize get_index_count() const {
		return indices.length() / sizeof(IndexType);
	}
};

struct MeshPushConst {
	glm::mat4 transform;
	glm::vec4 base_color;
};

struct Mesh : public utils::Move, public ptr::ToShared<Mesh> {
	std::vector<Primitive> primitives;

	static Mesh create_cube();
};