#pragma once

#include "external/VkBootstrap.h"

#include "utils/NoCopy.h"
#include "utils/move.h"
#include "utils/ptr.h"

#include "CommandManager.h"

#include "vk_resources/Image.h"

namespace vk {
	class SwapchainDeprecated : public utils::NoCopy {
	private:
		vkb::Swapchain swapchain;
		std::vector<ptr::Shared<Image>> _images;

	public:
		static SwapchainDeprecated create(const vkb::Device& device, const CommandManager& command_manager);
		static void destroy(SwapchainDeprecated& swapchain);

		void create_images();

		inline VkSwapchainKHR handle() const { return swapchain.swapchain; }

		inline uint32_t image_count() const { return swapchain.image_count; }
		inline const std::vector<ptr::Shared<Image>>& images() const { return _images; }

		inline VkFormat format() const { return swapchain.image_format; }
		inline VkExtent2D extent() const { return swapchain.extent; }
	};

	class Swapchain : public utils::Move, public ptr::ToShared<Swapchain> {
	private:
		Handle<VkSwapchainKHR> swapchain{};
		std::vector<ptr::Shared<Image>> _images;

	public:
		Swapchain() = default;
		inline ~Swapchain() {
			for (auto& image : _images) {
				*(image->image) = VK_NULL_HANDLE;
			}
			_images.clear();
		}

		inline VkSwapchainKHR handle() const { return *swapchain; }
	};
}