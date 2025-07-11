#pragma once

#include "vk/Context.h"

class App {
private:
	GLFWwindow* m_window;
	vk::Context m_context;

public:
	App();

	App(const App& other) = default;

	void run();
};