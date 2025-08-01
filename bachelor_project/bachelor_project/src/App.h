#pragma once

#include "external/glm.h"

#include "vk_core/Context.h"

#include "rendering/FrameManager.h"
#include "rendering/Rasterizer.h"

#include "scene/Camera.h"
#include "scene/Scene.h"

struct AppCamera : Camera {
	glm::vec3 center{ 0.0 };

	float distance{ 6.0f };
	float theta{ 0.0f };
	float phi{ 0.0f };

	float aspect{ 1.0f };
	float near{ 0.05f };
	float far{ 10.0f };

	CameraUBO as_camera_ubo() const override;
};

class App : public utils::Move {
private:
	GLFWwindow* window{};

	AppCamera camera{};

	FrameManager frame_manager{};

	ptr::Shared<Rasterizer> rasterizer{};
	ptr::Shared<Scene> scene{};

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