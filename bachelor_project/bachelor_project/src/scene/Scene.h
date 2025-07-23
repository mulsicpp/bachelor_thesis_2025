#pragma once

#include "Node.h"

#include <vector>
#include <string>

class Scene {
private:
	std::vector<ptr::Shared<Node>> nodes{};

public:
	Scene() = default;

	static Scene load(const std::string& file_path);
};