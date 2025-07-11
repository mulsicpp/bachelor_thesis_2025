#pragma once
#include "external/VkBootstrap.h" 
#include "utils/dbg_log.h"
#include "utils/NoCopy.h"
#include <GLFW/glfw3.h>

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

		vkb::Swapchain m_swapchain;

	public:
		Context();
		~Context();

		inline bool window_should_close() { return glfwWindowShouldClose(m_window); }
		inline void poll_events() { glfwPollEvents(); }
	};
}