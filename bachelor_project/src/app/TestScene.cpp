#include "TestScene.h"

#include "utils/AppPath.h"

std::string get_scene_name(TestScene test_scene) {
    switch (test_scene) {
    case TestScene::Brainstem: return "brainstem";
    case TestScene::Whirlwind: return "whirlwind";
    case TestScene::SpaceStation: return "space-station";
    default: return "monsters";
    }
}

std::string get_scene_path(TestScene test_scene) {
    const auto& app_path = utils::AppPath::instance();

    switch (test_scene) {
    case TestScene::Brainstem: return app_path.get_path("assets/scenes/BrainStem/glTF/BrainStem.gltf").string();
    case TestScene::Whirlwind: return app_path.get_path("assets/scenes/reap_the_whirlwind/scene.gltf").string();
    case TestScene::SpaceStation: return app_path.get_path("assets/scenes/space_station_3/scene.gltf").string();
    default: return app_path.get_path("assets/scenes/ultimate_monster_pack/scene.gltf").string();
    }
}

AppCamera get_scene_camera(TestScene test_scene) {
    AppCamera camera{};

    switch (test_scene) {
    case TestScene::Brainstem:
        camera.theta = glm::pi<float>() * 0.9f;
        camera.phi = -glm::pi<float>() * 0.1f;
        camera.center = glm::vec3{ 0.0f, -1.0f, 0.0f };
        camera.distance = 3;
        break;
    case TestScene::Whirlwind:
        camera.theta = glm::pi<float>() * 1.0f;
        camera.phi = 0.0f;
        camera.center = glm::vec3{ 0.0f, -400.0f, 0.0f };
        camera.distance = 1500;
        break;
    case TestScene::SpaceStation:
        camera.theta = 0.0f;
        camera.phi = 0.0f;
        camera.center = glm::vec3{ 0.0f, 0.0f, 0.0f };
        camera.distance = 10;
        break;
    case TestScene::Monsters:
        camera.theta = glm::pi<float>() * 1.0f;
        camera.phi = -glm::pi<float>() * 0.05f;
        camera.center = glm::vec3{ 0.0f, -1.0f, 0.0f };
        camera.distance = 40;
        break;
    }

    return camera;
}

glm::vec3 get_scene_light_dir(TestScene test_scene) {
    switch (test_scene) {
    case TestScene::Brainstem: return glm::vec3{0.2, -1.0, -0.7};
    case TestScene::Whirlwind: return glm::vec3{-0.2, -1.0, -0.7};
    case TestScene::SpaceStation: return glm::vec3{0.5, -1.0, 0.5};
    default: return glm::vec3{0.2, 0.0, -1.0};
    }
}
