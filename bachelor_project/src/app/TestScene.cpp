#include "TestScene.h"

#include "utils/AppPath.h"

std::string get_scene_name(TestScene test_scene) {
    switch (test_scene) {
    case TestScene::Brainstem: return "brainstem";
    case TestScene::Bathroom: return "bathroom";
    case TestScene::SpaceStation: return "space-station";
    case TestScene::Monsters: return "monsters";
    case TestScene::EternalValley: return "eternal-valley";
    default: return "brainstem";
    }
}

std::string get_scene_path(TestScene test_scene) {
    const auto& app_path = utils::AppPath::instance();

    switch (test_scene) {
    case TestScene::Brainstem: return app_path.get_path("assets/scenes/BrainStem/glTF/BrainStem.gltf").string();
    case TestScene::Bathroom: return app_path.get_path("assets/scenes/bathroom/scene.gltf").string();
    case TestScene::SpaceStation: return app_path.get_path("assets/scenes/space_station_3/scene.gltf").string();
    case TestScene::Monsters: return app_path.get_path("assets/scenes/ultimate_monster_pack/scene.gltf").string();
    case TestScene::EternalValley: return app_path.get_path("assets/scenes/eternal_valley/EternalValleyFPS_1.1.glb").string();
    default: return app_path.get_path("assets/scenes/BrainStem/glTF/BrainStem.gltf").string();
    }
}

AppCamera get_scene_camera(TestScene test_scene) {
    AppCamera camera{};

    switch (test_scene) {
    case TestScene::Brainstem:
        camera.theta = glm::pi<float>() * 0.1f;
        camera.phi = -glm::pi<float>() * 0.1f;
        camera.center = glm::vec3{ 0.0f, -1.0f, 0.0f };
        camera.distance = 3;
        break;
    case TestScene::Bathroom:
        camera.theta = -glm::pi<float>() * 0.75f;
        camera.phi = -glm::pi<float>() * 0.1f;
        camera.center = glm::vec3{ 3.6f, -0.9f, -3.6f };
        camera.distance = 1.0;
        break;
    case TestScene::SpaceStation:
        camera.theta = glm::pi<float>();
        camera.phi = 0.0f;
        camera.center = glm::vec3{ 0.0f, 0.0f, 0.0f };
        camera.distance = 10;
        break;
    case TestScene::Monsters:
        camera.theta = 0.0f;
        camera.phi = -glm::pi<float>() * 0.05f;
        camera.center = glm::vec3{ 0.0f, -2.0f, 12.0f };
        camera.distance = 28;
        break;
    case TestScene::EternalValley:
        camera.theta = glm::pi<float>() * 1.0f;
        camera.phi = -glm::pi<float>() * 0.03f;
        camera.center = glm::vec3{ -6.6f, -10.0f, -3.0f };
        camera.distance = 27;
        break;
    }

    return camera;
}

glm::vec3 get_scene_light_dir(TestScene test_scene) {
    switch (test_scene) {
    case TestScene::Brainstem: return glm::vec3{0.2, -1.0, -0.7};
    case TestScene::Bathroom: return glm::vec3{-0.2, -1.0, -0.7};
    case TestScene::SpaceStation: return glm::vec3{0.5, -1.0, 0.5};
    case TestScene::Monsters: return glm::vec3{0.2, 0.0, -1.0};
    default: return glm::vec3{0.3, -1.0, 1.0};
    }
}
