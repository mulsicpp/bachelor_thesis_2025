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
		static Context* context;

	public:
		GLFWwindow* window;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		vkb::Instance instance;

		vkb::PhysicalDevice physical_device;
		vkb::Device device;

		Swapchain swapchain;

		CommandManager command_manager;

		VmaAllocator allocator;

	public:
		static inline Context* create(GLFWwindow* window, const char* app_name) {
			if (context != nullptr)
				delete context;
			context = new Context(window, app_name);
			return context;
		}

		static inline void destroy() {
			if (context != nullptr)
				delete context;
		}

		static inline Context* get() {
			assert(context != nullptr);
			return context;
		}

	private:
		Context(GLFWwindow* window, const char* app_name);
		~Context();

		void create_allocator();
	};
}