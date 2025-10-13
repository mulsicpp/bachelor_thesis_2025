#pragma once

#include <glm/glm.hpp>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/CommandBuffer.h"
#include "vk_resources/SubBuffer.h"

#include "vk_pipeline/Pipeline.h"

#include "vk_rtx/Blas.h"

struct Material {
	static const ptr::Shared<Material> default_material;

	glm::vec3 base_color = glm::vec3{ 1.0f, 1.0f, 1.0f };
	float metallic_factor = 0.0f;
	float roughness_factor = 0.0f;
	
	glm::vec3 emissive_factor = glm::vec3{ 0.0f, 0.0f, 0.0f };
	float emissive_strength = 1.0f;

	uint32_t base_color_texture = ~uint32_t(0);
	uint32_t metallic_roughness_texture = ~uint32_t(0);
	uint32_t emission_texture = ~uint32_t(0);
	uint32_t normal_texture = ~uint32_t(0);
};

struct Node;

struct Skin {
	std::vector<glm::mat4> inverse_bind_matrices{};
	std::vector<ptr::Shared<Node>> nodes{};
	ptr::Shared<Node> skeleton_root{};
};

struct JointWeight {
	uint32_t index{};
	float weight{};
};

struct Primitive {
	using PositionType = glm::vec3;
	using UVType = glm::vec2;
	using ColorType = glm::vec<3, uint8_t>;

	using IndexType = uint32_t;

	std::vector<PositionType> positions_cpu{};
	std::vector<PositionType> normals_cpu{};
	std::vector<PositionType> tangents_cpu{};
	
	std::vector<std::vector<JointWeight>> joint_weights_cpu{};

	vk::SubBuffer positions{};
	vk::SubBuffer normals{};
	vk::SubBuffer tangents{};

	vk::SubBuffer uvs{};
	vk::SubBuffer joint_weights{};

	vk::SubBuffer indices{};


	enum class Topology {
		Triangles,
		TriangleStrip,
		TriangleFan,
	} topology{ Topology::Triangles };

	Material* material{};



	void draw(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, const glm::mat4& global_transform, vk::SubBuffer dynamic_positions = {}) const;
	void skin(vk::ReadyCommandBuffer cmd_buffer, vk::Pipeline* pipeline, vk::SubBuffer dynamic_positions, vk::SubBuffer joint_matrices, vk::SubBuffer dynamic_normals, vk::SubBuffer dynamic_tangents) const;

	static vk::VertexInput get_vertex_input();
	static vk::VertexInput get_skinning_vertex_input();

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

	vk::BlasGeometry get_blas_geometry(const vk::SubBuffer& dynamic_positions = {}) const;
};

struct MeshPushConst {
	glm::mat4 transform;
	glm::vec4 base_color;
};

struct SkinningPushConstant {
	uint32_t dynamic_positions_offset{};
	uint32_t dynamic_normals_offset{};
	uint32_t dynamic_tangents_offset{};
	uint32_t joint_matrices_offset{};
	uint32_t joint_weights_offset{};
	uint32_t joint_weights_per_vertex{};
};

struct Mesh : public utils::Move, public ptr::ToShared<Mesh> {
	std::vector<Primitive> primitives{};
	uint32_t primitive_offset_start{ 0 };

	ptr::Shared<vk::Blas> blas{};

	static Mesh create_cube();

	std::vector<vk::BlasGeometry> get_blas_geometries() const;

	void build_blas();
};