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

struct GLTFData {
	cgltf_data* data{};

	std::vector<ptr::Shared<Material>> materials{};
	std::vector<ptr::Shared<Mesh>> meshes{};
	std::vector<ptr::Shared<Skin>> skins{};

	std::vector<ptr::Shared<Node>> nodes{};

	std::vector<Animation> animations{};

	void create_materials();
	void create_meshes();

	void create_empty_nodes();
	void create_skins();

	void create_nodes();

	void create_animations();

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

	gltf.create_materials();
	gltf.create_meshes();

	gltf.create_empty_nodes();
	gltf.create_skins();
	gltf.create_nodes();

	gltf.create_animations();

	Scene scene{};
	scene.nodes = gltf.get_scene_nodes();
	scene.animations = std::move(gltf.animations);

	cgltf_free(gltf.data);

	return scene;
}

void Scene::update() {
	for (auto& node : nodes) {
		node->update_global_transfrom();
	}

	auto node_iter = iter();

	while (node_iter.has_next()) {
		const auto& node = node_iter.next();
		if (!node->mesh || !node->skin) {
			continue;
		}

		auto& skin = node->skin;

		std::vector<glm::mat4> joint_matrices{};
		for (uint32_t i = 0; i < skin->nodes.size(); i++) {
			joint_matrices.push_back(glm::inverse(node->global_transform) * skin->nodes[i]->global_transform * skin->inverse_bind_matrices[i]);
		}

		for (uint32_t i = 0; i < node->mesh->primitives.size(); i++) {
			auto& primitive = node->mesh->primitives[i];
			auto& dynamic_sub_buffer = node->dynamic_positions[i];

			auto* dynamic_positions = (glm::vec3*)(dynamic_sub_buffer.buffer()->mapped_data() + dynamic_sub_buffer.offset());
			uint32_t position_count = dynamic_sub_buffer.length() / sizeof(glm::vec3);


			for (uint32_t j = 0; j < position_count; j++) {
				glm::vec4 position = glm::vec4{ primitive.positions_cpu[j], 1.0f };

				const auto& joint_weights = primitive.joint_weights[j];

				glm::mat4 matrix = glm::mat4{};

				for (const auto& joint_weight : joint_weights) {
					matrix += joint_weight.weight * joint_matrices[joint_weight.index];
				}

				position = matrix * position;

				dynamic_positions[j].x = position.x;
				dynamic_positions[j].y = position.y;
				dynamic_positions[j].z = position.z;
			}
		}
		// dbg_log("updated dynamic positions");
	}
}



void GLTFData::create_materials() {
	materials.resize(data->materials_count);

	for (uint32_t i = 0; i < materials.size(); i++) {
		auto* gltf_material = &data->materials[i];

		Material material{};

		if (gltf_material->has_pbr_metallic_roughness) {
			material.base_color = glm::make_vec4(gltf_material->pbr_metallic_roughness.base_color_factor);
		}

		materials[i] = ptr::make_shared<Material>(material);
	}
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
			std::vector<cgltf_accessor*> joints{};
			std::vector<cgltf_accessor*> weights{};

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
				case cgltf_attribute_type_joints:
					if (attribute->index >= joints.size()) {
						joints.resize(attribute->index + 1, nullptr);
					}
					joints[attribute->index] = attribute->data;
					break;
				case cgltf_attribute_type_weights:
					if (attribute->index >= weights.size()) {
						weights.resize(attribute->index + 1, nullptr);
					}
					weights[attribute->index] = attribute->data;
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

			primitive.positions_cpu = position_data;

			primitive.positions = vk::SubBuffer::from(position_attr.buffer, position_attr.byte_offset(), element_count * sizeof(Primitive::PositionType));
			position_attr.data.insert(position_attr.data.cend(), position_data.begin(), position_data.end());

			primitive.uvs = vk::SubBuffer::from(garbage_attr.buffer, 0, element_count * sizeof(Primitive::UVType));
			primitive.colors = vk::SubBuffer::from(garbage_attr.buffer, 0, element_count * sizeof(Primitive::ColorType));

			if (garbage_attr.data.size() < element_count * sizeof(Primitive::UVType)) {
				garbage_attr.data.resize(element_count * sizeof(Primitive::UVType));
			}

			if (indices != nullptr) {
				VkDeviceSize index_count = cgltf_accessor_unpack_indices(indices, nullptr, sizeof(Primitive::IndexType), 0);
				std::vector<Primitive::IndexType> index_data{};
				index_data.resize(index_count);
				cgltf_accessor_unpack_indices(indices, index_data.data(), sizeof(Primitive::IndexType), index_count);

				primitive.indices = vk::SubBuffer::from(index_attr.buffer, index_attr.byte_offset(), index_count * sizeof(Primitive::IndexType));
				index_attr.data.insert(index_attr.data.cend(), index_data.begin(), index_data.end());
			}

			if (joints.size() < weights.size()) {
				joints.resize(weights.size(), nullptr);
			}
			else if (weights.size() < joints.size()) {
				weights.resize(joints.size(), nullptr);
			}

			if (joints.size() > 0) {

				primitive.joint_weights.resize(element_count, std::vector<JointWeight>{ 4 * joints.size() });

				std::vector<cgltf_uint> joint_data{};
				joint_data.resize(element_count * 4);
				std::vector<glm::vec4> weight_data{};
				weight_data.resize(element_count);

				for (uint32_t i = 0; i < joints.size(); i++) {
					for (uint32_t j = 0; j < element_count; j++) {
						cgltf_accessor_read_uint(joints[i], j, joint_data.data() + 4 * j, 4);
					}
					cgltf_accessor_unpack_floats(weights[i], (cgltf_float*)weight_data.data(), glm::vec4::length() * element_count);

					for (uint32_t k = 0; k < primitive.joint_weights.size(); k++) {
						for (uint32_t j = 0; j < 4; j++) {
							primitive.joint_weights[k][j + 4 * i].index = joint_data[k * 4 + j];
							primitive.joint_weights[k][j + 4 * i].weight = weight_data[k][j];
						}
					}
				}
			}

			if (gltf_primitive->material != nullptr) {
				primitive.material = materials[cgltf_material_index(data, gltf_primitive->material)];
			}
			else {
				primitive.material = Material::default_material;
			}

			mesh.primitives.emplace_back(std::move(primitive));
		}

		meshes[i] = std::move(mesh).to_shared();
	}

	auto buffer_builder = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
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

	if (index_attr.byte_offset() > 0) {
		*index_attr.buffer = buffer_builder
			.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
			.size(index_attr.byte_offset())
			.data(index_attr.data.data())
			.build();
	}
}

void GLTFData::create_empty_nodes() {
	nodes.resize(data->nodes_count);
	for (auto& node : nodes) {
		node = ptr::make_shared<Node>(Node{});
	}
}

void GLTFData::create_skins() {
	skins.resize(data->skins_count);

	for (uint32_t i = 0; i < skins.size(); i++) {
		auto* gltf_skin = &data->skins[i];

		Skin skin{};

		skin.nodes.resize(gltf_skin->joints_count);

		if (skin.nodes.size() == 0) {
			skins[i] = std::make_shared<Skin>(skin);
			continue;
		}

		if (gltf_skin->skeleton == nullptr) {
			std::vector<std::vector<cgltf_node*>> node_paths{ gltf_skin->joints_count };

			for (uint32_t j = 0; j < gltf_skin->joints_count; j++) {
				skin.nodes[j] = nodes[cgltf_node_index(data, gltf_skin->joints[j])];

				std::vector<cgltf_node*> node_path{};

				node_path.push_back(gltf_skin->joints[j]);
				while(node_path.back()->parent != nullptr) {
					node_path.push_back(node_path.back()->parent);
				}

				node_paths[j] = node_path;
			}

			cgltf_node* root = nullptr;
			bool root_found = false;
			while (!root_found) {

				for (uint32_t i = 1; i < node_paths.size(); i++) {
					if (node_paths[i].empty() || node_paths[i - 1].empty() || node_paths[i].back() != node_paths[i - 1].back()) {
						root_found = true;
						break;
					}
				}

				if (root_found) {
					break;
				}

				root = node_paths[0].back();

				for (auto& node_path : node_paths) {
					node_path.pop_back();
				}
			}

			skin.skeleton_root = nodes[cgltf_node_index(data, root)];
		}
		else {

			for (uint32_t j = 0; j < skin.nodes.size(); j++) {
				skin.nodes[j] = nodes[cgltf_node_index(data, gltf_skin->joints[j])];
			}

			skin.skeleton_root = nodes[cgltf_node_index(data, gltf_skin->skeleton)];
		}

		std::vector<glm::mat4> inverse_bind_matrices{ };
		inverse_bind_matrices.resize(skin.nodes.size());
		if (gltf_skin->inverse_bind_matrices != nullptr) {
			cgltf_accessor_unpack_floats(gltf_skin->inverse_bind_matrices, (cgltf_float*)inverse_bind_matrices.data(), 16 * inverse_bind_matrices.size());
		}
		else {
			for (auto& mat : inverse_bind_matrices) {
				mat = glm::mat4{ 1.0f };
			}
		}

		skin.inverse_bind_matrices = inverse_bind_matrices;

		skins[i] = ptr::make_shared<Skin>(skin);

		dbg_log("skin added");
	}
}

void GLTFData::create_nodes() {
	VkDeviceSize dynamic_buffer_size = 0;
	ptr::Shared<vk::Buffer> dynamic_buffer = ptr::make_shared<vk::Buffer>();

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
			if (gltf_node->skin != nullptr) {
				node.skin = skins[cgltf_skin_index(data, gltf_node->skin)];

				for (const auto& primitive : node.mesh->primitives) {
					node.dynamic_positions.push_back(vk::SubBuffer::from(dynamic_buffer, dynamic_buffer_size, primitive.positions.length()));
					dynamic_buffer_size += primitive.positions.length();
				}
			}
		}

		node.children = std::vector<ptr::Shared<Node>>{ gltf_node->children_count };
		for (uint32_t j = 0; j < gltf_node->children_count; j++) {
			uint32_t child_index = gltf_node->children[j] - data->nodes;

			dbg_log("node %u has child %u", i, child_index);
			node.children[j] = nodes[child_index];
		}
		*nodes[i] = std::move(node);
	}

	*dynamic_buffer = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
		.size(dynamic_buffer_size)
		.build();
}



void GLTFData::create_animations() {
	animations.resize(data->animations_count);

	for (uint32_t i = 0; i < animations.size(); i++) {
		auto* gltf_animation = &data->animations[i];

		Animation animation{};

		for (uint32_t j = 0; j < gltf_animation->channels_count; j++) {
			auto* gltf_channel = &gltf_animation->channels[j];
			auto* gltf_sampler = gltf_channel->sampler;

			Interpolation interpolation{};
			switch (gltf_sampler->interpolation) {
			case cgltf_interpolation_type_step:
				interpolation = Interpolation::Step;
				break;
			case cgltf_interpolation_type_linear:
				interpolation = Interpolation::Linear;
				break;
			case cgltf_interpolation_type_cubic_spline:
				interpolation = Interpolation::CubicSpline;
				break;
			}

			std::vector<float> input{};
			std::vector<float> output{};

			input.resize(cgltf_accessor_unpack_floats(gltf_sampler->input, nullptr, 0));
			output.resize(cgltf_accessor_unpack_floats(gltf_sampler->output, nullptr, 0));

			cgltf_accessor_unpack_floats(gltf_sampler->input, input.data(), input.size());
			cgltf_accessor_unpack_floats(gltf_sampler->output, output.data(), output.size());

			float* output_data = output.data();

			ptr::Owned<AnimationChannel> channel{};

			const ptr::Shared<Node>& node = nodes[cgltf_node_index(data, gltf_channel->target_node)];

			switch (gltf_channel->target_path) {
			case cgltf_animation_path_type_translation:
			case cgltf_animation_path_type_scale:
			{
				Sampler<glm::vec3> sampler{};
				sampler.interpolation = interpolation;

				for (uint32_t i = 0; i < input.size(); i++) {
					sampler.samples.push_back({ input[i], glm::make_vec3(output_data + 3 * i) });
				}

				if (gltf_channel->target_path == cgltf_animation_path_type_scale) {
					channel = std::make_unique<ScaleChannel>(node, sampler);
				}
				else {
					channel = std::make_unique<TranslationChannel>(node, sampler);
				}

				break;
			}
			case cgltf_animation_path_type_rotation:
			{
				Sampler<glm::quat> sampler{};
				sampler.interpolation = interpolation;

				for (uint32_t i = 0; i < input.size(); i++) {
					sampler.samples.push_back({ input[i], glm::make_quat(output_data + 4 * i) });
				}

				channel = std::make_unique<RotationChannel>(node, sampler);

				break;
			}
			}

			animation.channels.emplace_back(std::move(channel));
		}

		animations[i] = std::move(animation);
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