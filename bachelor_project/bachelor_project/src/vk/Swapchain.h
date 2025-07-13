#pragma once

#include "external/VkBootstrap.h"

namespace vk {
	class Swapchain {
	private:
		vkb::Swapchain m_swapchain;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_image_views;

	public:
		static Swapchain create(vkb::Device device);
		static void destroy(Swapchain& swapchain);
	};
}