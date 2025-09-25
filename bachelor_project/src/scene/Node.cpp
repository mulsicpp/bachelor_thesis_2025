#include "Node.h"

void Node::update_global_transfrom(glm::mat4 parent_transform) {
	if (transform.raw) {
		global_transform = parent_transform * transform.transform.raw.to_mat();
	}
	else {
		global_transform = parent_transform * transform.transform.matrix;
	}

	for (auto& child : children) {
		child->update_global_transfrom(global_transform);
	}
}

std::vector<vk::BlasGeometry> Node::get_dynamic_blas_geometries() const {
	std::vector<vk::BlasGeometry> geometries{};
	geometries.reserve(mesh->primitives.size());

	for (uint32_t i = 0; i < mesh->primitives.size(); i++) {
		geometries.push_back(mesh->primitives[i].get_blas_geometry(dynamic_positions[i]));
	}

	return geometries;
}

void Node::build_blas(bool fast_build) {
	if(!mesh) {
		return;
	}

	if (skin && !dynamic_positions.empty()) {
		blas = vk::BlasBuilder()
			.geometries(mesh->get_blas_geometries())
			.dynamic(true)
			.fast_build(fast_build)
			.build().to_shared();
	}
	else {
		if (!mesh->blas) {
			mesh->build_blas();
		}
		blas = mesh->blas;
	}
}

void Node::rebuild_blas() {
	if(!mesh || !skin || dynamic_positions.empty() || !blas) {
		return;
	}

	blas->rebuild(get_dynamic_blas_geometries());
}

void Node::refit_blas() {
	if(!mesh || !skin || dynamic_positions.empty() || !blas) {
		return;
	}

	blas->refit(get_dynamic_blas_geometries());
}