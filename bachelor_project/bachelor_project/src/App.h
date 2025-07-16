#pragma once

#include "vk_core/Context.h"

#include "Rasterizer.h"

class App {
private:
	GLFWwindow* window;

	Rasterizer rasterizer;

public:
	App();
	~App();

	void run();
};