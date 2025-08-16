#pragma once

#include "Node.h"
#include "Animation.h"

#include <vector>
#include <string>

class Scene {
private:
	std::vector<ptr::Shared<Skin>> skins{};
	std::vector<ptr::Shared<Node>> nodes{};
	std::vector<Animation> animations{};

	VkDeviceSize dynamic_buffer_size{ 0 };
	ptr::Shared<vk::Buffer> dynamic_buffer{};

public:
	Scene() = default;

	inline Animation& get_animation(uint32_t index) { return animations[index]; };

	static Scene load(const std::string& file_path);

	void update();

	inline NodeIterator iter() {
		return NodeIterator::from(nodes);
	}

	inline VkDeviceSize get_dynamic_buffer_size() const {
		return dynamic_buffer_size;
	}

	inline void select_dynamic_buffer(const ptr::Shared<vk::Buffer>& dynamic_buffer) {
		this->dynamic_buffer = dynamic_buffer;
	}
};