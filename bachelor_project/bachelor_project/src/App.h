#pragma once

#include "vk_core/Context.h"

class App {
private:
	GLFWwindow* window;

public:
	App();
	~App();

	void run();
};