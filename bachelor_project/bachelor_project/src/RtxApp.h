#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/Raytracer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

class RtxApp : public utils::Move {
private:
	Raytracer raytracer{};

	ptr::Shared<vk::Image> color_image{};

	AppCamera camera{};
	ptr::Shared<Scene> scene{};

public:
	RtxApp();

	void run();
};