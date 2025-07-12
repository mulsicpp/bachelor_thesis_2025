#pragma once

#include "vk/Context.h"

class App {
private:
	GLFWwindow* m_window;
	std::unique_ptr<vk::Context> m_context;

public:
	App();
	~App();

	void run();
};