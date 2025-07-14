#pragma once

#include "vk_core/Context.h"

class App {
private:
	GLFWwindow* window;
	std::unique_ptr<vk::Context> context;

public:
	App();
	~App();

	void run();
};