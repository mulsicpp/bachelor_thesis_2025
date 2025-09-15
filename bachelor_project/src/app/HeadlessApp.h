#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/Rasterizer.h"
#include "rendering/Skinner.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include <chrono>

class HeadlessApp : public utils::Move {
private:
	ptr::Shared<Rasterizer> rasterizer{};
	Skinner skinner{};
	
	ptr::Shared<vk::Image> color_image{};
	ptr::Shared<vk::Image> depth_image{};
	
	vk::Framebuffer framebuffer{};
	
	ptr::Shared<AppCamera> camera{};
	ptr::Shared<Scene> scene{};

public:
	HeadlessApp();

	void run();
};