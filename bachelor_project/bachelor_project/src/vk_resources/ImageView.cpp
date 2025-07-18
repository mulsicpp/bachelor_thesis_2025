#include "ImageView.h"

#include "vk_core/Context.h"

namespace vk {
	ImageView ImageView::create_from(Image&& image, VkImageAspectFlags aspect) {
        return create_from(std::move(image).to_shared(), aspect);
	}

    ImageView ImageView::create_from(const ptr::Shared<const Image>& image, VkImageAspectFlags aspect) {
        ImageView image_view;

        image_view._image = image;
        image_view._aspect = aspect;

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = image->handle();
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = image->format();
        view_info.subresourceRange.aspectMask = aspect;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(Context::get()->get_device(), &view_info, nullptr, &*image_view.image_view) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return image_view;
    }
}