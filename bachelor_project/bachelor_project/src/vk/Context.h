#pragma once
#include "external/VkBootstrap.h" 
#include "utils/dbg_log.h"
#include "utils/NoCopy.h"

#include "Swapchain.h"
#include "CommandManager.h"

#include <GLFW/glfw3.h>

#include <memory>

namespace vk {

	class Context : utils::NoCopy {
	private:
		GLFWwindow* m_window;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;

		vkb::Instance m_instance;

		vkb::PhysicalDevice m_physical_device;
		vkb::Device m_device;

		// TODO handle queues
		VkQueue m_graphics_queue = VK_NULL_HANDLE;
		VkQueue m_transfer_queue = VK_NULL_HANDLE;
		VkQueue m_compute_queue = VK_NULL_HANDLE;
		VkQueue m_present_queue = VK_NULL_HANDLE;

		vk::Swapchain m_swapchain;

		vk::CommandManager m_command_manager;

	public:
		static inline std::unique_ptr<Context> create(GLFWwindow* window, const char* app_name) {
			return std::unique_ptr<Context>(new Context(window, app_name));
		}

		~Context();
	private:
		Context(GLFWwindow* window, const char* app_name);
	};
}