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