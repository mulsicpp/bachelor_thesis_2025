#pragma once

#include "external/glm.h"

#include <vector>

#include "utils/ptr.h"

#include "Mesh.h"

struct RawTransform {
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;

	inline glm::mat4 to_mat() {
		return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
	}
};

union NodeTransform {
	glm::mat4 matrix;
	RawTransform raw;
};

struct ModelUBO {
	glm::mat4 transform;
};

class Node {
private:
	bool raw_transform{ false };
	NodeTransform transform{};
	glm::mat4 global_transform{ 1.0 };
	std::vector<ptr::Shared<Node>> children{};
	ptr::Shared<Mesh> mesh{};

	void update_global_transfrom(glm::mat4 parent_transform = { 1.0f });

	ModelUBO as_model_ubo() const { return ModelUBO{ global_transform }; }
};