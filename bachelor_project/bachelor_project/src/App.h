#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

#include "scene/Camera.h"

struct AppCamera : Camera {
	glm::vec3 center{ 0.0 };

	float distance{ 4.0f };
	float theta{ 0.0f };
	float phi{ 0.0f };

	CameraUBO as_camera_ubo() const override;
};

class App : public utils::Move {
private:
	GLFWwindow* window{};

	AppCamera camera{};

	FrameManager frame_manager{};

	ptr::Shared<Rasterizer> rasterizer{};

public:
	App();
	~App();

	void run();
};