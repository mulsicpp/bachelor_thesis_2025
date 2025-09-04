#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/Raytracer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include "CLIOptions.h"

class RtxApp : public utils::Move {
private:
	CLIOptions opts{};

	Raytracer raytracer{};

	ptr::Shared<vk::Image> image{};
	ptr::Shared<vk::ImageView> image_view{};

	ptr::Shared<AppCamera> camera{};
	ptr::Shared<Scene> scene{};

public:
	RtxApp(int argc, char* argv[]);

	void run();
};