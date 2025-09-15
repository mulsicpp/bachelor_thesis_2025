#pragma once

#include <string>
#include "scene/Camera.h"

enum class TestScene {
    Brainstem,
    Whirlwind,
    SpaceStation,
    Monsters
};

std::string get_scene_name(TestScene test_scene);
std::string get_scene_path(TestScene test_scene);
AppCamera get_scene_camera(TestScene test_scene);