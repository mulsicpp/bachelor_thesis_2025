#pragma once

#include "external/vk_mem_alloc.h"
#include "external/VkBootstrap.h"

#include "utils/dbg_log.h"
#include "utils/NoCopy.h"
#include "utils/ptr_alias.h"

#include "Swapchain.h"
#include "CommandManager.h"

#include <GLFW/glfw3.h>

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

		VmaAllocator allocator;

	public:
		static inline ptr::Owned<Context> create(GLFWwindow* window, const char* app_name) {
			return ptr::Owned<Context>(new Context(window, app_name));
		}

		~Context();
	private:
		Context(GLFWwindow* window, const char* app_name);

		void create_allocator();
	};
}