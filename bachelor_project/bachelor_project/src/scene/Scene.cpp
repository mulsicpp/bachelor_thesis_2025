#include "Scene.h"

#include "external/cgltf.h"

#include <glm/gtc/type_ptr.hpp>

#include "utils/dbg_log.h"
#include "utils/load_file.h"

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

	std::vector<ptr::Shared<vk::Buffer>> buffers{};
	std::vector<ptr::Shared<Node>> nodes{};

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

	gltf.create_nodes();

	Scene scene{};
	scene.nodes = gltf.get_scene_nodes();

	cgltf_free(gltf.data);

	return scene;
}



void GLTFData::create_nodes() {
	nodes.resize(data->nodes_count);
	for (auto& node : nodes) {
		node = ptr::make_shared<Node>(Node{});
	}

	for (uint32_t i = 0; i < nodes.size(); i++) {
		Node node{};
		cgltf_node& gltf_node = data->nodes[i];

		if (gltf_node.has_matrix) {
			node.transform = NodeTransform{ glm::make_mat4(gltf_node.matrix) };
		}
		else {
			RawTransform raw{};
			if (gltf_node.has_translation) {
				raw.translation = glm::make_vec3(gltf_node.translation);
			}
			if (gltf_node.has_rotation) {
				raw.rotation = glm::make_quat(gltf_node.rotation);
			}
			if (gltf_node.has_scale) {
				raw.scale = glm::make_vec3(gltf_node.scale);
			}
			node.transform = NodeTransform{ raw };
		}


		node.children = std::vector<ptr::Shared<Node>>{ gltf_node.children_count };
		for (uint32_t j = 0; j < gltf_node.children_count; j++) {
			uint32_t child_index = gltf_node.children[j] - data->nodes;

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