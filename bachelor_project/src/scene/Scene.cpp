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

#define OFFSET_VALUE_MASK 0x7fffffff
#define OFFSET_FLAG_MASK 0x80000000
#define OFFSET_ATTR_UNUSED (~(uint32_t)0)

struct PrimitiveOffset {
	bool dyn;
	uint32_t positions;
	uint32_t normals;
	uint32_t tangents;
	uint32_t uvs;
	uint32_t indices;
};

struct GLTFData {
	cgltf_data* data{};

	std::vector<Material> materials{};
	std::vector<ptr::Shared<Mesh>> meshes{};
	std::vector<ptr::Shared<Skin>> skins{};

	std::vector<ptr::Shared<Node>> nodes{};

	std::vector<Animation> animations{};

	SceneBuffers buffers{};

	std::vector<PrimitiveOffset> primitive_offsets{};

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
	scene.materials = std::move(gltf.materials);

	scene.buffers = gltf.buffers;
	cgltf_free(gltf.data);

	return scene;
}

void Scene::update_transforms() {
	for (auto& node : nodes) {
		node->update_global_transfrom();
	}
}

void Scene::skin_cpu() {
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
			auto* dynamic_normals = (node->dynamic_normals.size() > 0 && node->dynamic_normals[i].buffer()) ? (glm::vec3*)(node->dynamic_normals[i].buffer()->mapped_data() + node->dynamic_normals[i].offset()) : nullptr;
			auto* dynamic_tangents = (node->dynamic_tangents.size() > 0 && node->dynamic_tangents[i].buffer()) ? (glm::vec3*)(node->dynamic_tangents[i].buffer()->mapped_data() + node->dynamic_tangents[i].offset()) : nullptr;
			uint32_t position_count = dynamic_sub_buffer.length() / sizeof(glm::vec3);


			for (uint32_t j = 0; j < position_count; j++) {
				glm::vec4 position = glm::vec4{ primitive.positions_cpu[j], 1.0f };

				const auto& joint_weights = primitive.joint_weights_cpu[j];

				glm::mat4 matrix = glm::mat4{};

				for (const auto& joint_weight : joint_weights) {
					matrix += joint_weight.weight * joint_matrices[joint_weight.index];
				}

				position = matrix * position;

				dynamic_positions[j].x = position.x;
				dynamic_positions[j].y = position.y;
				dynamic_positions[j].z = position.z;

				glm::mat3 normal_matrix = glm::transpose(glm::inverse(matrix));

				if(dynamic_normals) {
					dynamic_normals[j] = normal_matrix * primitive.normals_cpu[j];
				}

				if(dynamic_tangents) {
					dynamic_tangents[j] = normal_matrix * primitive.tangents_cpu[j];
				}
			}
		}
	}
}

void Scene::build_acceleration_structures(bool fast_build) {
	NodeIterator it = iter();

	auto tlas_builder = vk::TlasBuilder().dynamic(true).fast_build(fast_build);

	int32_t next_instance_idx = 0;

	while (it.has_next()) {
		const auto node = it.next();

		if (node->mesh) {
			node->build_blas(fast_build);

			node->instance_index = next_instance_idx++;

			vk::TlasInstance tlas_instance{};
			tlas_instance.blas = node->blas;
			tlas_instance.transform = node->global_transform;
			tlas_instance.custom_index = node->mesh->primitive_offset_start;

			tlas_builder.add_instance(tlas_instance);
		}
		else {
			node->instance_index = -1;
		}
	}

	tlas = tlas_builder.build().to_shared();
}

void Scene::rebuild_acceleration_structures(double* update_time) {
	NodeIterator it = iter();

	auto& tlas_instances = tlas->instances();

	while (it.has_next()) {
		const auto node = it.next();

		if (!node->blas) {
			continue;
		}

		node->rebuild_blas(update_time);

		auto& tlas_instance = tlas_instances[node->instance_index];
		tlas_instance.blas = node->blas;
		tlas_instance.transform = node->global_transform;
		tlas_instance.custom_index = node->skin ? node->dyn_primitive_offset_start : node->mesh->primitive_offset_start;
	}

	if (update_time) {
		*update_time += tlas->rebuild();
	}
}

void Scene::refit_acceleration_structures(double* update_time) {
	NodeIterator it = iter();

	auto& tlas_instances = tlas->instances();

	while (it.has_next()) {
		const auto node = it.next();

		if (!node->blas) {
			continue;
		}

		node->refit_blas(update_time);

		auto& tlas_instance = tlas_instances[node->instance_index];
		tlas_instance.blas = node->blas;
		tlas_instance.transform = node->global_transform;
		tlas_instance.custom_index = node->skin ? node->dyn_primitive_offset_start : node->mesh->primitive_offset_start;
	}

	if (update_time) {
		*update_time += tlas->refit();
	}
}



void GLTFData::create_materials() {
	materials.resize(data->materials_count);

	for (uint32_t i = 0; i < materials.size(); i++) {
		auto* gltf_material = &data->materials[i];

		Material material{};

		material.emissive_factor = glm::make_vec3(gltf_material->emissive_factor);

		if (gltf_material->has_pbr_metallic_roughness) {
			material.base_color = glm::make_vec3(gltf_material->pbr_metallic_roughness.base_color_factor);
			material.metallic_factor = gltf_material->pbr_metallic_roughness.metallic_factor;
			material.roughness_factor = gltf_material->pbr_metallic_roughness.roughness_factor;
		}

		if(gltf_material->has_emissive_strength) {
			material.emissive_strength = gltf_material->emissive_strength.emissive_strength;
		}

		materials[i] = material;
	}
}

template<class T>
struct AttributeData {
	ptr::Shared<vk::Buffer> buffer{ ptr::make_shared<vk::Buffer>(vk::Buffer{}) };
	std::vector<T> data{};

	inline VkDeviceSize byte_offset() const { return data.size() * sizeof(T); }
	inline VkDeviceSize element_size() const { return sizeof(T); }

	inline vk::SubBuffer append_elements(cgltf_accessor* accessor, uint32_t count, std::vector<T>* p_vec = nullptr) {
		if (!accessor) {
			return vk::SubBuffer{};
		}

		std::vector<T> vec{};
		vec.resize(count);
		cgltf_accessor_unpack_floats(accessor, (cgltf_float*)vec.data(), T::length() * count);

		if (p_vec) {
			*p_vec = vec;
		}

		auto sub_buffer = vk::SubBuffer::from(buffer, byte_offset(), count * sizeof(T));
		data.insert(data.cend(), vec.begin(), vec.end());

		return sub_buffer;
	}
};

#ifdef _WIN32

static const VkBufferUsageFlags VERTEX_BUFFER_USAGES =
VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

static const VkBufferUsageFlags INDEX_BUFFER_USAGES =
VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

#else

static const VkBufferUsageFlags VERTEX_BUFFER_USAGES =
VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

static const VkBufferUsageFlags INDEX_BUFFER_USAGES =
VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

#endif

void GLTFData::create_meshes() {
	meshes.resize(data->meshes_count);

	AttributeData<Primitive::PositionType> position_attr{};
	AttributeData<Primitive::UVType> uv_attr{};
	AttributeData<Primitive::IndexType> index_attr{};

	VkDeviceSize joint_weights_buffer_size = 0;
	buffers.joint_weights = ptr::make_shared<vk::Buffer>();

	for (uint32_t i = 0; i < meshes.size(); i++) {
		auto* gltf_mesh = &data->meshes[i];
		Mesh mesh{};

		mesh.primitive_offset_start = primitive_offsets.size();

		for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++) {
			auto* gltf_primitive = &gltf_mesh->primitives[j];
			if (gltf_primitive->type != cgltf_primitive_type_triangles) {
				continue;
			}

			Primitive primitive{};
			primitive.topology = Primitive::Topology::Triangles;


			cgltf_accessor* indices = gltf_primitive->indices;

			cgltf_accessor* positions = nullptr;
			cgltf_accessor* normals = nullptr;
			cgltf_accessor* tangents = nullptr;

			cgltf_accessor* uvs = nullptr;
			std::vector<cgltf_accessor*> joints{};
			std::vector<cgltf_accessor*> weights{};

			for (uint32_t k = 0; k < gltf_primitive->attributes_count; k++) {
				auto* attribute = &gltf_primitive->attributes[k];
				switch (attribute->type) {
				case cgltf_attribute_type_position:
					positions = attribute->data;
					break;
				case cgltf_attribute_type_normal:
					normals = attribute->data;
					break;
				case cgltf_attribute_type_tangent:
					tangents = attribute->data;
					break;
				case cgltf_attribute_type_texcoord:
					if (attribute->index == 0)
						uvs = attribute->data;
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

			primitive.positions = position_attr.append_elements(positions, element_count, &primitive.positions_cpu);
			primitive.normals = position_attr.append_elements(normals, element_count, &primitive.normals_cpu);
			primitive.tangents = position_attr.append_elements(tangents, element_count, &primitive.tangents_cpu);

			primitive.uvs = uv_attr.append_elements(uvs, element_count);

			if (indices != nullptr) {
				VkDeviceSize index_count = cgltf_accessor_unpack_indices(indices, nullptr, sizeof(Primitive::IndexType), 0);
				std::vector<Primitive::IndexType> index_data{};
				index_data.resize(index_count);
				cgltf_accessor_unpack_indices(indices, index_data.data(), sizeof(Primitive::IndexType), index_count);

				primitive.indices = vk::SubBuffer::from(index_attr.buffer, index_attr.byte_offset(), index_count * sizeof(Primitive::IndexType));
				index_attr.data.insert(index_attr.data.cend(), index_data.begin(), index_data.end());
			}

			PrimitiveOffset primitive_offset{};
			primitive_offset.dyn = false;
			primitive_offset.positions = primitive.positions.offset() / sizeof(Primitive::PositionType);
			primitive_offset.normals = normals ? primitive.normals.offset() / sizeof(Primitive::PositionType) : OFFSET_ATTR_UNUSED;
			primitive_offset.tangents = tangents ? primitive.tangents.offset() / sizeof(Primitive::PositionType) : OFFSET_ATTR_UNUSED;
			primitive_offset.uvs = uvs ? primitive.uvs.offset() / sizeof(Primitive::UVType) : OFFSET_ATTR_UNUSED;
			primitive_offset.indices = indices ? primitive.indices.offset() / sizeof(Primitive::IndexType) : OFFSET_ATTR_UNUSED;

			primitive_offsets.push_back(primitive_offset);

			if (joints.size() < weights.size()) {
				joints.resize(weights.size(), nullptr);
			}
			else if (weights.size() < joints.size()) {
				weights.resize(joints.size(), nullptr);
			}

			if (joints.size() > 0) {
				uint32_t element_size = 4 * joints.size();
				primitive.joint_weights_cpu.resize(element_count, std::vector<JointWeight>{ element_size });

				primitive.joint_weights = vk::SubBuffer::from(buffers.joint_weights, joint_weights_buffer_size, element_count * element_size * sizeof(JointWeight));
				joint_weights_buffer_size += primitive.joint_weights.length();

				std::vector<cgltf_uint> joint_data{};
				joint_data.resize(element_count * 4);
				std::vector<glm::vec4> weight_data{};
				weight_data.resize(element_count);

				for (uint32_t i = 0; i < joints.size(); i++) {
					for (uint32_t j = 0; j < element_count; j++) {
						cgltf_accessor_read_uint(joints[i], j, joint_data.data() + 4 * j, 4);
					}
					cgltf_accessor_unpack_floats(weights[i], (cgltf_float*)weight_data.data(), glm::vec4::length() * element_count);

					for (uint32_t k = 0; k < primitive.joint_weights_cpu.size(); k++) {
						for (uint32_t j = 0; j < 4; j++) {

							JointWeight value = { joint_data[k * 4 + j], weight_data[k][j] };

							primitive.joint_weights_cpu[k][4 * i + j] = value;
						}
					}
				}
			}

			if (gltf_primitive->material != nullptr) {
				primitive.material = &materials[cgltf_material_index(data, gltf_primitive->material)];
			}
			else {
				primitive.material = &*Material::default_material;
			}

			mesh.primitives.emplace_back(std::move(primitive));
		}

		meshes[i] = std::move(mesh).to_shared();
	}

	auto buffer_builder = vk::BufferBuilder()
		.usage(VERTEX_BUFFER_USAGES | VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer, vk::QueueType::Compute });

	*position_attr.buffer = buffer_builder
		.size(position_attr.byte_offset())
		.data(position_attr.data.data())
		.build();

	buffers.positions = position_attr.buffer;

	if (uv_attr.byte_offset() > 0) {
		*uv_attr.buffer = buffer_builder
			.size(uv_attr.byte_offset())
			.data(uv_attr.data.data())
			.build();

		buffers.uvs = uv_attr.buffer;
	}

	if (index_attr.byte_offset() > 0) {
		*index_attr.buffer = buffer_builder
			.usage(INDEX_BUFFER_USAGES)
			.size(index_attr.byte_offset())
			.data(index_attr.data.data())
			.build();

		buffers.indices = index_attr.buffer;
	}

	printf("position count: %u\n", position_attr.byte_offset());
	printf("uv count: %u\n", uv_attr.byte_offset());
	printf("index count: %u\n", index_attr.byte_offset());

	auto joint_weights_staging_buffer = vk::BufferBuilder().staging_buffer().size(joint_weights_buffer_size).build();


	for (const auto& mesh : meshes) {
		for (const auto& primitive : mesh->primitives) {
			JointWeight* joint_weight_data = (JointWeight*)(joint_weights_staging_buffer.mapped_data() + primitive.joint_weights.offset());
			uint32_t offset = 0;
			for (const auto& vertex_weights : primitive.joint_weights_cpu) {
				for (const auto& weight : vertex_weights) {
					joint_weight_data[offset++] = weight;
				}
			}
		}
	}

	*buffers.joint_weights = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer, vk::QueueType::Compute })
		.size(joint_weights_buffer_size)
		.build();

	joint_weights_staging_buffer.copy_into(&*buffers.joint_weights);
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
				while (node_path.back()->parent != nullptr) {
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
	}
}

void GLTFData::create_nodes() {
	VkDeviceSize dynamic_buffer_size = 0;
	buffers.dynamic_positions = ptr::make_shared<vk::Buffer>();

	VkDeviceSize joint_matrix_buffer_size = 0;
	buffers.joint_matrices = ptr::make_shared<vk::Buffer>();


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

		node.dyn_primitive_offset_start = primitive_offsets.size();

		if (gltf_node->mesh != nullptr) {
			node.mesh = meshes[cgltf_mesh_index(data, gltf_node->mesh)];
			if (gltf_node->skin != nullptr) {
				node.skin = skins[cgltf_skin_index(data, gltf_node->skin)];

				node.joint_matrices = vk::SubBuffer::from(buffers.joint_matrices, joint_matrix_buffer_size, sizeof(glm::mat4) * node.skin->nodes.size());

				joint_matrix_buffer_size += node.joint_matrices.length();

				for (auto& primitive : node.mesh->primitives) {
					auto dynamic_positions = vk::SubBuffer::from(buffers.dynamic_positions, dynamic_buffer_size, primitive.positions.length());
					node.dynamic_positions.push_back(dynamic_positions);
					dynamic_buffer_size += primitive.positions.length();

					vk::SubBuffer dynamic_normals, dynamic_tangents;

					if (primitive.normals.buffer()) {
						dynamic_normals = vk::SubBuffer::from(buffers.dynamic_positions, dynamic_buffer_size, primitive.normals.length());
						node.dynamic_normals.push_back(dynamic_normals);
						dynamic_buffer_size += primitive.normals.length();
					}

					if (primitive.tangents.buffer()) {
						dynamic_tangents = vk::SubBuffer::from(buffers.dynamic_positions, dynamic_buffer_size, primitive.tangents.length());
						node.dynamic_tangents.push_back(dynamic_tangents);
						dynamic_buffer_size += primitive.tangents.length();
					}

					PrimitiveOffset primitive_offset{};
					primitive_offset.dyn = true;
					primitive_offset.positions = dynamic_positions.offset() / sizeof(Primitive::PositionType);
					primitive_offset.normals = primitive.normals.buffer() ? dynamic_normals.offset() / sizeof(Primitive::PositionType) : OFFSET_ATTR_UNUSED;;
					primitive_offset.tangents = primitive.tangents.buffer() ? dynamic_tangents.offset() / sizeof(Primitive::PositionType) : OFFSET_ATTR_UNUSED;
					primitive_offset.uvs = primitive.uvs.buffer() ? primitive.uvs.offset() / sizeof(Primitive::UVType) : OFFSET_ATTR_UNUSED;
					primitive_offset.indices = primitive.indices.buffer() ? primitive.indices.offset() / sizeof(Primitive::IndexType) : OFFSET_ATTR_UNUSED;

					primitive_offsets.push_back(primitive_offset);
				}
			}
		}

		node.children = std::vector<ptr::Shared<Node>>{ gltf_node->children_count };
		for (uint32_t j = 0; j < gltf_node->children_count; j++) {
			uint32_t child_index = gltf_node->children[j] - data->nodes;
			node.children[j] = nodes[child_index];
		}
		*nodes[i] = std::move(node);
	}

	*buffers.dynamic_positions = vk::BufferBuilder()
		.usage(VERTEX_BUFFER_USAGES)
		.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer, vk::QueueType::Compute })
		.size(dynamic_buffer_size)
		.build();

	*buffers.joint_matrices = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		.memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer, vk::QueueType::Compute })
		.size(joint_matrix_buffer_size)
		.build();

	buffers.primitive_offsets = vk::BufferBuilder()
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		.memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
		.queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer, vk::QueueType::Compute })
		.size(primitive_offsets.size() * sizeof(PrimitiveOffset))
		.data(primitive_offsets.data())
		.build().to_shared();
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