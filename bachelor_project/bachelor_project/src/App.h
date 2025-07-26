#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

#include "scene/Camera.h"

struct AppCamera : Camera {
	glm::vec3 center;

	float distance;
	float theta;
	float phi;

	CameraUBO as_camera_ubo() const override;
};

class App : public utils::Move {
private:
	GLFWwindow* window;

	FrameManager frame_manager;

	ptr::Shared<Rasterizer> rasterizer;

public:
	App();
	~App();

	void run();
};