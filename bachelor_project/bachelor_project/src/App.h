#pragma once

#include "vk_core/Context.h"

class App {
private:
	GLFWwindow* window;
	ptr::Owned<vk::Context> context;

public:
	App();
	~App();

	void run();
};