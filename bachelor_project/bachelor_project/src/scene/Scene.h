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

	inline void update() { for (auto& node : nodes) node->update_global_transfrom(); }
	inline NodeIterator iter() {
		return NodeIterator::from(nodes);
	}
};