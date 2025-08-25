#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

#include <chrono>

class App : public utils::Move {
private:
	GLFWwindow* window{};

	AppCamera camera{};

	FrameManager frame_manager{};

	ptr::Shared<Rasterizer> rasterizer{};
	ptr::Shared<Scene> scene{};

	Frame frame{};

	std::chrono::high_resolution_clock::time_point time{};

public:
	App();
	~App();

	void run();

	static inline void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
		app->frame_manager.signal_resize();
		app->camera.aspect = ((float)width) / ((float)height);
	}

	static void scroll_callback(GLFWwindow* window, double x_offset, double y_offset);
	static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};