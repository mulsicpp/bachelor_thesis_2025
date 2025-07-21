#pragma once

#include <glm/glm.hpp>
#include <vector>

class Node {
private:
	glm::mat4 transform{ 1.0f };
	std::vector<uint32_t> child_indices{};
	uint32_t mesh_index{ UINT32_MAX };
};