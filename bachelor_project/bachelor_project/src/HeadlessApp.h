#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include <chrono>

class HeadlessApp : public utils::Move {
private:
	ptr::Shared<Rasterizer> rasterizer{};
	ptr::Shared<Scene> scene{};

	ptr::Shared<vk::Buffer> dynamic_buffer{};

public:
	HeadlessApp();

	void run();
};