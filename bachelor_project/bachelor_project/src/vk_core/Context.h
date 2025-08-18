#pragma once

#include "external/VolkBootstrap.h"
#include "external/volk_mem_alloc.h"

#include "utils/dbg_log.h"
#include "utils/NoCopy.h"
#include "utils/ptr.h"

#include "Swapchain.h"
#include "CommandManager.h"

#include <GLFW/glfw3.h>

#include "external/volk.h"

namespace vk {

	class ContextInfo;

	class Context : public utils::NoCopy {
	private:
		static Context* context;

		GLFWwindow* window;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		vkb::Instance instance;
		vkb::PhysicalDevice physical_device;
		vkb::Device device;

		CommandManager command_manager;

		VmaAllocator allocator;

	public:
		static inline Context* create(const ContextInfo& info) {
			if (context != nullptr)
				delete context;
			context = new Context(info);
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
		VmaAllocator get_allocator() const { return allocator; }

		void wait_device_idle() const { vkDeviceWaitIdle(device.device); }

	private:
		Context(const ContextInfo& info);
		~Context();

		void create_allocator();
	};


	class ContextInfo {
		friend class Context;
	public:
		using Ref = ContextInfo&;

	private:
		GLFWwindow* _window{ nullptr };
		std::string _app_name{ "" };
		bool _use_debugging{ DEBUG_VULKAN };
		bool _use_raytracing{ false };

	public:
		ContextInfo() = default;

		inline Ref window(GLFWwindow* window) { _window = window; return *this; }
		inline Ref app_name(const std::string& app_name) { _app_name = app_name; return *this; }
		inline Ref use_debugging(bool use_debugging) { _use_debugging = use_debugging; return *this; }
		inline Ref use_raytracing(bool use_raytracing) { _use_raytracing = use_raytracing; return *this; }
	};
}