#pragma once

#include "Node.h"
#include "Animation.h"

#include <vector>
#include <string>

#include "vk_rtx/Tlas.h"

class Scene {
private:
	std::vector<ptr::Shared<Skin>> skins{};
	std::vector<ptr::Shared<Node>> nodes{};
	std::vector<Animation> animations{};

	vk::Tlas tlas{};

public:
	Scene() = default;

	inline Animation& get_animation(uint32_t index) { return animations[index]; };

	static Scene load(const std::string& file_path);

	void update();

	void build_acceleration_structures();

	inline NodeIterator iter() {
		return NodeIterator::from(nodes);
	}
};