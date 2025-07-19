#pragma once

#include "external/vk_mem_alloc.h"
#include "external/VkBootstrap.h"

#include "utils/dbg_log.h"
#include "utils/NoCopy.h"
#include "utils/ptr.h"

#include "Swapchain.h"
#include "CommandManager.h"

#include <GLFW/glfw3.h>

namespace vk {

	class Context : public utils::NoCopy {
	private:
		static Context* context;

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
			context->swapchain.create_images();
			return context;
		}

		static inline void destroy() {
			if (context != nullptr)
				delete context;
			context = nullptr;
		}

		static inline Context* get() {
			if (context == nullptr) {
				throw std::runtime_error("No Vulkan context present");
			}
			return context;
		}

		static inline Context* get_noexcept() noexcept {
			return context;
		}

		GLFWwindow* get_window() const { return window; }
		VkSurfaceKHR get_surface() const { return surface; }

		VkInstance get_instance() const { return instance.instance; }
		VkPhysicalDevice get_physical_device() const { return physical_device.physical_device; }
		VkDevice get_device() const { return device.device; }

		const CommandManager& get_command_manager() const { return command_manager; }
		const Swapchain& get_swapchain() const { return swapchain; }
		VmaAllocator get_allocator() const { return allocator; }

		void wait_device_idle() const { vkDeviceWaitIdle(device.device); }

	private:
		Context(GLFWwindow* window, const char* app_name);
		~Context();

		void create_allocator();
	};
}