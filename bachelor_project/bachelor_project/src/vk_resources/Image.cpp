#include "Image.h"
#include "vk_core/Context.h"

namespace vk {

	Image::Image()
		: image{ VK_NULL_HANDLE }
		, allocation { VK_NULL_HANDLE }
		, image_view { VK_NULL_HANDLE }
	{}
	
	void Image::destroy() {
		const auto& context = *Context::get();

		vkDestroyImageView(context.get_device(), image_view, nullptr);
		vmaDestroyImage(context.get_allocator(), image, allocation);
	}
}