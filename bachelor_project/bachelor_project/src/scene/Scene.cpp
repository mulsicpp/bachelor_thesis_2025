#include "Scene.h"

#include "external/json.h"

#include "utils/dbg_log.h"

#include <fstream>
#include <filesystem>

using json = nlohmann::json;

Scene Scene::load(const std::string& file_path) {
	Scene scene;

	auto old_path = std::filesystem::canonical(std::filesystem::current_path());


	std::ifstream file(file_path);
	json data = json::parse(file);

	std::filesystem::current_path(std::filesystem::path(file_path).parent_path());

	auto scene_idx = data["scene"].get<uint32_t>();
	for (const auto& scene : data["scenes"]) {
		dbg_log("scene:")
		for (const auto& node : scene["nodes"]) {
			dbg_log("    node: %u", node.get<uint32_t>());
		}
	}

	std::filesystem::current_path(old_path);

	return scene;
}