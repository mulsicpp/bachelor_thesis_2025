#pragma once

#include "external/VkBootstrap.h"
#include "CommandManager.h"

namespace vk {
	class Swapchain {
	private:
		vkb::Swapchain swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;

	public:
		static Swapchain create(const vkb::Device& device, const CommandManager& command_manager);
		static void destroy(Swapchain& swapchain);
	};
}