#pragma once

#include "external/VkBootstrap.h"

#include "utils/NoCopy.h"

#include "CommandManager.h"

#include "vk_resources/Image.h"

namespace vk {
	class Swapchain : public utils::NoCopy {
	private:
		vkb::Swapchain swapchain;
		std::vector<ptr::Shared<Image>> _images;

	public:
		static Swapchain create(const vkb::Device& device, const CommandManager& command_manager);
		static void destroy(Swapchain& swapchain);

		void create_images();

		inline VkSwapchainKHR handle() const { return swapchain.swapchain; }

		inline uint32_t image_count() const { return swapchain.image_count; }
		inline const std::vector<ptr::Shared<Image>>& images() const { return _images; }

		inline VkFormat format() const { return swapchain.image_format; }
		inline VkExtent2D extent() const { return swapchain.extent; }
	};
}