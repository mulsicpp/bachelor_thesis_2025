#pragma once

#include "external/vk_mem_alloc.h"
#include "external/VkBootstrap.h"

#include "utils/dbg_log.h"
#include "utils/NoCopy.h"

#include "Swapchain.h"
#include "CommandManager.h"
#include "Allocator.h"

#include <GLFW/glfw3.h>

#include <memory>

namespace vk {

	class Context : utils::NoCopy {
	private:
		GLFWwindow* window;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		vkb::Instance instance;

		vkb::PhysicalDevice physical_device;
		vkb::Device device;

		Swapchain swapchain;

		CommandManager command_manager;

		Allocator allocator;

	public:
		static inline std::unique_ptr<Context> create(GLFWwindow* window, const char* app_name) {
			return std::unique_ptr<Context>(new Context(window, app_name));
		}

		~Context();
	private:
		Context(GLFWwindow* window, const char* app_name);
	};
}