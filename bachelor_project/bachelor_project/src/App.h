#pragma once

#include "vk_core/Context.h"

#include "rendering/Rasterizer.h"

class App : public utils::Move {
private:
	GLFWwindow* window;

	Rasterizer rasterizer;

public:
	App();
	~App();

	void run();
};