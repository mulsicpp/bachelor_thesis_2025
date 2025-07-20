#include "Swapchain.h"

#include "Context.h"

#include <algorithm>

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

	static VkSurfaceFormatKHR get_surface_format(VkPhysicalDevice device, VkSurfaceKHR surface, VkFormat format, VkColorSpaceKHR color_space) {
		std::vector<VkSurfaceFormatKHR> formats{};
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

		if (format_count != 0) {
			formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, formats.data());
		}

		for (const auto& surface_format : formats) {
			if (surface_format.format == format && surface_format.colorSpace == color_space) {
				return surface_format;
			}
		}

		return formats[0];
	}

	static VkPresentModeKHR get_present_mode(VkPhysicalDevice device, VkSurfaceKHR surface, VkPresentModeKHR present_mode) {
		std::vector<VkPresentModeKHR> present_modes{};
		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

		if (present_mode_count != 0) {
			present_modes.resize(present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, present_modes.data());
		}

		for (const auto& mode : present_modes) {
			if (mode == present_mode) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	Swapchain SwapchainBuilder::build() {
		Swapchain swapchain;

		const auto& context = *Context::get();
		const auto physical_device = context.get_physical_device();
		const auto surface = context.get_surface();

		auto surface_format = get_surface_format(physical_device, surface, _desired_format, _desired_color_space);
		auto present_mode = get_present_mode(physical_device, surface, _desired_present_mode);

		VkSurfaceCapabilitiesKHR capabilities;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

		VkExtent2D extent;
		if (capabilities.currentExtent.width != UINT32_MAX) {
			extent = capabilities.currentExtent;
		}
		else {
			_extent.width = std::clamp(_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			_extent.height = std::clamp(_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			extent = _extent;
		}

		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
			image_count = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;

		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const auto& cmd_manager = context.get_command_manager();

		auto families = cmd_manager.get_required_families({ QueueType::Graphics, QueueType::Present });

		create_info.imageSharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = families.size();
		create_info.pQueueFamilyIndices = families.data();

		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;

		const auto device = context.get_device();

		if (vkCreateSwapchainKHR(device, &create_info, nullptr, &*swapchain.swapchain) != VK_SUCCESS) {
			throw std::runtime_error("Swapchain creation failed!");
		}

		vkGetSwapchainImagesKHR(device, swapchain.swapchain.get(), &image_count, nullptr);
		std::vector<VkImage> swapchain_image_handles{ image_count };
		vkGetSwapchainImagesKHR(device, swapchain.swapchain.get(), &image_count, swapchain_image_handles.data());

		swapchain._surface_format = surface_format;
		swapchain._extent = extent;

		swapchain._images = {};

		for (const auto image_handle : swapchain_image_handles) {
			Image image;

			*image.allocation = VK_NULL_HANDLE;
			*image.image = { image_handle, false };

			image._format = surface_format.format;
			image._extent = extent;
			swapchain._images.push_back(std::move(image).to_shared());
		}

		return swapchain;
	}
}