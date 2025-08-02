#pragma once

#include "Node.h"
#include "Animation.h"

#include <vector>
#include <string>

class Scene {
private:
	std::vector<ptr::Shared<Node>> nodes{};
	std::vector<Animation> animations{};

public:
	Scene() = default;

	inline Animation& get_animation(uint32_t index) { return animations[index]; };

	static Scene load(const std::string& file_path);

	inline void update() { for (auto& node : nodes) node->update_global_transfrom(); }
	inline NodeIterator iter() {
		return NodeIterator::from(nodes);
	}
};