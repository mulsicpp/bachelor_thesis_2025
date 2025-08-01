#include "Scene.h"

#include "external/cgltf.h"

#include <glm/gtc/type_ptr.hpp>

#include "utils/dbg_log.h"
#include "utils/load_file.h"

#include "vk_resources/SubBuffer.h"

#include <fstream>
#include <filesystem>
#include <stdexcept>

const std::vector<const char*> GLTF_ERROR_TEXTS = {
	"success",
	"data_too_short",
	"unknown_format",
	"invalid_json",
	"invalid_gltf",
	"invalid_options",
	"file_not_found",
	"io_error",
	"out_of_memory",
	"legacy_gltf",
};

static struct GLTFData {
	cgltf_data* data{};

	std::vector<ptr::Shared<Mesh>> meshes{};

	std::vector<ptr::Shared<Node>> nodes{};

	void create_meshes();
	void create_nodes();
	std::vector<ptr::Shared<Node>> get_scene_nodes();
};

Scene Scene::load(const std::string& file_path) {
	cgltf_options options = {};
	GLTFData gltf;

	cgltf_result result = cgltf_parse_file(&options, file_path.c_str(), &gltf.data);
	if (result != cgltf_result_success)
	{
		throw std::runtime_error("GLTF parsing failed for file '" + file_path + "': " + GLTF_ERROR_TEXTS[result]);
	}

	result = cgltf_load_buffers(&options, gltf.data, file_path.c_str());

	if (result != cgltf_result_success)
	{
		throw std::runtime_error("GLTF buffer loading failed for '" + file_path + "': " + GLTF_ERROR_TEXTS[result]);
	}

	gltf.create_meshes();
	gltf.create_nodes();

	Scene scene{};
	scene.nodes = gltf.get_scene_nodes();

	cgltf_free(gltf.data);

	return scene;
}



template<class T>
struct AttributeData {
	ptr::Shared<vk::Buffer> buffer{ ptr::make_shared<vk::Buffer>(vk::Buffer{}) };
	std::vector<T> data{};

	inline VkDeviceSize byte_offset() const { return data.size() * sizeof(T); }
	inline VkDeviceSize element_size() const { return sizeof(T); }
};

void GLTFData::create_meshes() {
	meshes.resize(data->meshes_count);

	AttributeData<Primitive::PositionType> position_attr;
	AttributeData<Primitive::UVType> uv_attr;
	AttributeData<Primitive::ColorType> color_attr;
	AttributeData<Primitive::IndexType> index_attr;

	AttributeData<uint8_t> garbage_attr;

	for (uint32_t i = 0; i < meshes.size(); i++) {
		auto* gltf_mesh = &data->meshes[i];
		Mesh mesh{};

		for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++) {
			auto* gltf_primitive = &gltf_mesh->primitives[j];
			if (gltf_primitive->type != cgltf_primitive_type_triangles) {
				dbg_log("meshes[%u].primitives[%u].type != triangles! Ignored!", i, j);
				continue;
			}

			Primitive primitive{};
			primitive.topology = Primitive::Topology::Triangles;


			cgltf_accessor* indices = gltf_primitive->indices;

			cgltf_accessor* positions = nullptr;
			cgltf_accessor* uvs = nullptr;
			cgltf_accessor* colors = nullptr;
			
			for (uint32_t k = 0; k < gltf_primitive->attributes_count; k++) {
				auto* attribute = &gltf_primitive->attributes[k];
				switch (attribute->type) {
				case cgltf_attribute_type_position:
					positions = attribute->data;
					break;
				case cgltf_attribute_type_texcoord:
					uvs = attribute->data;
					break;
				case cgltf_attribute_type_color:
					colors = attribute->data;
					break;
				}
			}

			if (positions == nullptr) {
				dbg_log("meshes[%u].primitives[%u]: position attribute is nullptr! Ignored!", i, j);
				continue;
			}

			if (positions->component_type != cgltf_component_type_r_32f || positions->type != cgltf_type_vec3) {
				dbg_log("meshes[%u].primitives[%u]: position attribute is not vec3! Ignored!", i, j);
				continue;
			}

			VkDeviceSize element_count = positions->count;
			std::vector<Primitive::PositionType> position_data{ element_count };
			cgltf_accessor_unpack_floats(positions, (cgltf_float*)position_data.data(), Primitive::PositionType::length() * element_count);
			
			primitive.positions = vk::SubBuffer::from(position_attr.buffer, position_attr.byte_offset(), element_count * sizeof(Primitive::PositionType));
			position_attr.data.insert(position_attr.data.cend(), position_data.begin(), position_data.end());
			
			primitive.uvs = vk::SubBuffer::from(garbage_attr.buffer, 0, element_count * sizeof(Primitive::UVType));
			primitive.colors = vk::SubBuffer::from(garbage_attr.buffer, 0, element_count * sizeof(Primitive::ColorType));
			
			if (garbage_attr.data.size() < element_count * sizeof(Primitive::UVType)) {
				garbage_attr.data.resize(element_count * sizeof(Primitive::UVType));
			}


			VkDeviceSize index_count = cgltf_accessor_unpack_indices(indices, nullptr, sizeof(Primitive::IndexType), 0);
			std::vector<Primitive::IndexType> index_data{};
			index_data.resize(index_count);
			cgltf_accessor_unpack_indices(indices, index_data.data(), sizeof(Primitive::IndexType), index_count);
			
			primitive.indices = vk::SubBuffer::from(index_attr.buffer, index_attr.byte_offset(), index_count * sizeof(Primitive::IndexType));
			index_attr.data.insert(index_attr.data.cend(), index_data.begin(), index_data.end());
			
			mesh.primitives.emplace_back(std::move(primitive));
		}

		meshes[i] = std::move(mesh).to_shared();
	}

	auto buffer_builder = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer });
	
	*position_attr.buffer = buffer_builder
		.size(position_attr.byte_offset())
		.data(position_attr.data.data())
		.build();
	
	memset(garbage_attr.data.data(), 0, garbage_attr.byte_offset());
	*garbage_attr.buffer = buffer_builder
		.size(garbage_attr.byte_offset())
		.data(garbage_attr.data.data())
		.build();
	
	*index_attr.buffer = buffer_builder
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		.size(index_attr.byte_offset())
		.data(index_attr.data.data())
		.build();
}

void GLTFData::create_nodes() {
	nodes.resize(data->nodes_count);
	for (auto& node : nodes) {
		node = ptr::make_shared<Node>(Node{});
	}

	for (uint32_t i = 0; i < nodes.size(); i++) {
		Node node{};
		cgltf_node* gltf_node = &data->nodes[i];

		if (gltf_node->has_matrix) {
			node.transform = NodeTransform{ glm::make_mat4(gltf_node->matrix) };
		}
		else {
			RawTransform raw{};
			if (gltf_node->has_translation) {
				raw.translation = glm::make_vec3(gltf_node->translation);
			}
			if (gltf_node->has_rotation) {
				raw.rotation = glm::make_quat(gltf_node->rotation);
			}
			if (gltf_node->has_scale) {
				raw.scale = glm::make_vec3(gltf_node->scale);
			}
			node.transform = NodeTransform{ raw };
		}

		if (gltf_node->mesh != nullptr) {
			node.mesh = meshes[cgltf_mesh_index(data, gltf_node->mesh)];
		}


		node.children = std::vector<ptr::Shared<Node>>{ gltf_node->children_count };
		for (uint32_t j = 0; j < gltf_node->children_count; j++) {
			uint32_t child_index = gltf_node->children[j] - data->nodes;

			dbg_log("node %u has child %u", i, child_index);
			node.children[j] = nodes[child_index];
		}
		*nodes[i] = std::move(node);
	}
}

std::vector<ptr::Shared<Node>> GLTFData::get_scene_nodes() {
	if (data->scene == nullptr) {
		dbg_log("No scene in gltf");
		return {};
	}
	std::vector<ptr::Shared<Node>> scene_nodes{ data->scene->nodes_count };

	for (uint32_t i = 0; i < scene_nodes.size(); i++) {
		uint32_t node_index = data->scene->nodes[i] - data->nodes;
		scene_nodes[i] = nodes[node_index];
	}

	return scene_nodes;
}