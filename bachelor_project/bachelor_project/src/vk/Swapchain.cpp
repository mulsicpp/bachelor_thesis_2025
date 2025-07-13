#include "Swapchain.h"

namespace vk {
	Swapchain Swapchain::create(vkb::Device device) {
		Swapchain swapchain;

		vkb::SwapchainBuilder swapchain_builder(device);

		auto swapchain_result = swapchain_builder.build();
		if (!swapchain_result) {
			throw std::runtime_error("Swapchain creation failed! " + swapchain_result.error().message());
		}
		swapchain.m_swapchain = swapchain_result.value();

		auto images_result = swapchain.m_swapchain.get_images();
		if (!images_result) {
			throw std::runtime_error("Swapchain images aquisition failed! " + images_result.error().message());
		}
		swapchain.m_images = images_result.value();

		auto image_views_result = swapchain.m_swapchain.get_image_views();
		if (!image_views_result) {
			throw std::runtime_error("Swapchain images aquisition failed! " + image_views_result.error().message());
		}
		swapchain.m_image_views = image_views_result.value();

		return swapchain;
	}

	void Swapchain::destroy(Swapchain& swapchain) {
		swapchain.m_swapchain.destroy_image_views(swapchain.m_image_views);
		vkb::destroy_swapchain(swapchain.m_swapchain);
	}
}