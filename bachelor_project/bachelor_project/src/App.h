#pragma once

#include "vk_core/Context.h"

#include "renderer/Rasterizer.h"

class App : public utils::Move {
private:
	GLFWwindow* window;

	Rasterizer rasterizer;

public:
	App();
	~App();

	void run();
};