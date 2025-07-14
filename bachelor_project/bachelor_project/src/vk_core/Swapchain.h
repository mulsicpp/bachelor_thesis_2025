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

		VkSwapchainKHR handle() const {
			return swapchain.swapchain;
		}

		const std::vector<VkImage>& get_images() const {
			return images;
		}

		const std::vector<VkImageView>& get_image_views() const {
			return image_views;
		}
	};
}