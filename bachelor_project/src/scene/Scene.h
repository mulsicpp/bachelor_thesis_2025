#pragma once

#include "Node.h"
#include "Animation.h"
#include "SceneBuffers.h"

#include <vector>
#include <string>

#include "vk_rtx/Tlas.h"

class Scene {
private:
	std::vector<Texture> textures{};
	std::vector<Material> materials{};
	std::vector<ptr::Shared<Skin>> skins{};
	std::vector<ptr::Shared<Node>> nodes{};
	std::vector<Animation> animations{};

	ptr::Shared<vk::Tlas> tlas{};

	SceneBuffers buffers{};

public:
	Scene() = default;

	inline NodeIterator iter() {
		return NodeIterator::from(nodes);
	}

	inline uint32_t animation_count() const { return animations.size(); }
	inline Animation& get_animation(uint32_t index) { return animations[index]; }
	inline const ptr::Shared<vk::Tlas>& get_tlas() { return tlas; }
	inline const std::vector<Texture>& get_textures() { return textures; }

	inline const SceneBuffers& get_buffers() { return buffers; }

	static Scene load(const std::string& file_path);

	void update_transforms();
	void skin_cpu();

	void build_acceleration_structures(bool fast_build);
	void rebuild_acceleration_structures(double* update_time = nullptr);
	void refit_acceleration_structures(double* update_time = nullptr);
};