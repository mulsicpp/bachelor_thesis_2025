#include "Image.h"
#include "vk_core/Context.h"

namespace vk {

	Image ImageBuilder::build() const {
		Image image;

        const auto& context = *Context::get();

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = _extent.width;
        image_info.extent.height = _extent.height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = _format;
        image_info.tiling = _tiling;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = _usage;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;

        const auto families = context.get_command_manager().get_required_families(_queue_types);
        image_info.sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        image_info.queueFamilyIndexCount = static_cast<uint32_t>(families.size());
        image_info.pQueueFamilyIndices = families.data();

        VmaAllocationCreateInfo allocation_info{};
        allocation_info.usage = _memory_usage;

        if (vmaCreateImage(context.get_allocator(), &image_info, &allocation_info, &(*image.image).handle, &*image.allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Image creation failed!");
        }

        image._extent = _extent;
        image._format = _format;

		return image;
	}
}