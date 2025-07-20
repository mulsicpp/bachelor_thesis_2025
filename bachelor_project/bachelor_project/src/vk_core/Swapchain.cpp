#include "Swapchain.h"

namespace vk {
	SwapchainDeprecated SwapchainDeprecated::create(const vkb::Device& device, const CommandManager& command_manager) {
		SwapchainDeprecated swapchain;

		vkb::SwapchainBuilder swapchain_builder{
			device.physical_device.physical_device,
			device.device, device.surface,
			command_manager[QueueType::Graphics].family_index,
			command_manager[QueueType::Present].family_index
		};

		auto swapchain_result = swapchain_builder.build();
		if (!swapchain_result) {
			throw std::runtime_error("Swapchain creation failed! " + swapchain_result.error().message());
		}
		swapchain.swapchain = swapchain_result.value();

		swapchain._images = {};

		return swapchain;
	}

	void SwapchainDeprecated::create_images() {
		_images = {};

		auto images_result = swapchain.get_images();
		if (!images_result) {
			throw std::runtime_error("Swapchain images aquisition failed! " + images_result.error().message());
		}

		for (const auto raw_image : images_result.value()) {
			Image image;

			*image.allocation = VK_NULL_HANDLE;
			*image.image = raw_image;

			image._format = swapchain.image_format;
			image._extent = swapchain.extent;

			_images.push_back(std::move(image).to_shared());
		}
	}

	void SwapchainDeprecated::destroy(SwapchainDeprecated& swapchain) {

		for (auto& image : swapchain._images) {
			*(image->image) = VK_NULL_HANDLE;
		}
		swapchain._images.clear();
		vkb::destroy_swapchain(swapchain.swapchain);
	}
}