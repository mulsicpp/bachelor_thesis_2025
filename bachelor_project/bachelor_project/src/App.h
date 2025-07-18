#pragma once

#include "vk_core/Context.h"

#include "renderer/Rasterizer.h"

class App {
private:
	GLFWwindow* window;

	Rasterizer rasterizer;

public:
	App();
	~App();

	void run();
};