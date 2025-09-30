#pragma once

#include "external/glm.h"

#include "scene/Camera.h"

#include <string>

enum class TestScene {
    Brainstem,
    Whirlwind,
    SpaceStation,
    Monsters,
    EternalValley
};

std::string get_scene_name(TestScene test_scene);
std::string get_scene_path(TestScene test_scene);
AppCamera get_scene_camera(TestScene test_scene);

glm::vec3 get_scene_light_dir(TestScene test_scene);