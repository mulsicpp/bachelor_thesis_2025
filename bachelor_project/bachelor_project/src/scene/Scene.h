#pragma once

#include "Node.h"

#include <vector>
#include <string>

class Scene {
private:
	std::vector<Node> nodes{};
	uint32_t root_index{ UINT32_MAX };

public:
	static Scene load(const std::string& file_path);
};