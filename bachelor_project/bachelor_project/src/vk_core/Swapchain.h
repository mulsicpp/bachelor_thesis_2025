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

	class SwapchainBuilder;

	class Swapchain : public utils::Move, public ptr::ToShared<Swapchain> {
		friend class SwapchainBuilder;
	private:
		Handle<VkSwapchainKHR> swapchain{};
		std::vector<ptr::Shared<Image>> _images{};

		VkSurfaceFormatKHR _surface_format{};
		VkExtent2D _extent{};

	public:
		Swapchain() = default;

		inline VkSwapchainKHR handle() const { return *swapchain; }

		inline uint32_t image_count() const { return _images.size(); }
		inline const std::vector<ptr::Shared<Image>>& images() const { return _images; }

		inline VkSurfaceFormatKHR surface_format() const { return _surface_format; }
		inline VkExtent2D extent() const { return _extent; }
	};

	class SwapchainBuilder {
	public:
		using Ref = SwapchainBuilder&;

	private:
		VkFormat _desired_format{ VK_FORMAT_R8G8B8A8_SRGB };
		VkColorSpaceKHR _desired_color_space{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		VkPresentModeKHR _desired_present_mode{ VK_PRESENT_MODE_MAILBOX_KHR };
		VkExtent2D _extent{ 0, 0 };
	
	public:
		SwapchainBuilder() = default;

		inline Ref desired_format(VkFormat desired_format) { _desired_format = desired_format; return *this; }
		inline Ref desired_color_space(VkColorSpaceKHR desired_color_space) { _desired_color_space = desired_color_space; return *this; }
		inline Ref desired_present_mode(VkPresentModeKHR desired_present_mode) { _desired_present_mode = desired_present_mode; return *this; }
		inline Ref extent(VkExtent2D extent) { _extent = extent; return *this; }

		Swapchain build();
	};
}