#pragma once

#include "external/VkBootstrap.h"

#include "utils/NoCopy.h"

#include "CommandManager.h"

namespace vk {
	class Swapchain : public utils::NoCopy {
	private:
		vkb::Swapchain swapchain;
		std::vector<VkImage> images;
		std::vector<VkImageView> image_views;

	public:
		static Swapchain create(const vkb::Device& device, const CommandManager& command_manager);
		static void destroy(Swapchain& swapchain);

		inline VkSwapchainKHR handle() const { return swapchain.swapchain; }

		inline uint32_t get_image_count() const { return swapchain.image_count; }
		inline const std::vector<VkImage>& get_images() const { return images; }
		inline const std::vector<VkImageView>& get_image_views() const { return image_views; }

		inline VkFormat get_format() const { return swapchain.image_format; }
		inline VkExtent2D get_extent() const { return swapchain.extent; }
	};
}