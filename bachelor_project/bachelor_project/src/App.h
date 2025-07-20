#pragma once

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

class App : public utils::Move {
private:
	GLFWwindow* window;

	FrameManager frame_manager;

	Rasterizer rasterizer;

public:
	App();
	~App();

	void run();
};