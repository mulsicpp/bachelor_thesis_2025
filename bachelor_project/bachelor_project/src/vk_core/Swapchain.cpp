#include "Swapchain.h"

namespace vk {
	Swapchain Swapchain::create(const vkb::Device& device, const CommandManager& command_manager) {
		Swapchain swapchain;

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

		auto images_result = swapchain.swapchain.get_images();
		if (!images_result) {
			throw std::runtime_error("Swapchain images aquisition failed! " + images_result.error().message());
		}
		swapchain.images = images_result.value();

		auto image_views_result = swapchain.swapchain.get_image_views();
		if (!image_views_result) {
			throw std::runtime_error("Swapchain images aquisition failed! " + image_views_result.error().message());
		}
		swapchain.image_views = image_views_result.value();

		return swapchain;
	}

	void Swapchain::destroy(Swapchain& swapchain) {
		swapchain.swapchain.destroy_image_views(swapchain.image_views);
		vkb::destroy_swapchain(swapchain.swapchain);
	}
}