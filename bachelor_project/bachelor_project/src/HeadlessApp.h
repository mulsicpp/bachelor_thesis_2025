#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/Rasterizer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include <chrono>

class HeadlessApp : public utils::Move {
private:
	ptr::Shared<Rasterizer> rasterizer{};
	Frame frame{};

	ptr::Shared<vk::Image> color_image{};
	ptr::Shared<vk::Image> depth_image{};

	vk::Framebuffer framebuffer{};

	AppCamera camera{};
	ptr::Shared<Scene> scene{};

public:
	HeadlessApp();

	void run();
};