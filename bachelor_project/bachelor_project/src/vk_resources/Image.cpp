#include "Image.h"
#include "vk_core/Context.h"

namespace vk
{

    struct ImageStateInfo {
		VkImageLayout layout;
		VkAccessFlags access_mask;
		VkPipelineStageFlags stage_mask;
	};

    static const std::vector<ImageStateInfo> IMAGE_STATE_INFOS = {
        {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
        {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
        {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
        {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}
    };


    void Image::cmd_transition(ReadyCommandBuffer cmd_buffer, ImageState src_state, ImageState dst_state) {
        const auto& src_info = IMAGE_STATE_INFOS[(uint32_t)src_state];
        const auto& dst_info = IMAGE_STATE_INFOS[(uint32_t)dst_state];

        VkImageMemoryBarrier memory_barrier{};
        memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        
        memory_barrier.image = (*image).handle;

        memory_barrier.oldLayout = src_info.layout;
        memory_barrier.newLayout = dst_info.layout;

        memory_barrier.srcAccessMask = src_info.access_mask;
        memory_barrier.dstAccessMask = dst_info.access_mask;

        memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        memory_barrier.subresourceRange.aspectMask = _aspect;
        memory_barrier.subresourceRange.layerCount = 1;
        memory_barrier.subresourceRange.baseArrayLayer = 0;
        memory_barrier.subresourceRange.levelCount = 1;
        memory_barrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(cmd_buffer.handle(), src_info.stage_mask, dst_info.stage_mask, 0, 0, nullptr, 0, nullptr, 1, &memory_barrier);
    }


    void Image::cmd_load(ReadyCommandBuffer cmd_buffer, Buffer *buffer, const std::vector<VkBufferImageCopy> &copy_regions)
    {
        if (copy_regions.size() > 0)
        {
            vkCmdCopyBufferToImage(cmd_buffer.handle(), buffer->handle(), (*image).handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(copy_regions.size()), copy_regions.data());
            return;
        }

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {_extent.width, _extent.height, 1};

        vkCmdCopyBufferToImage(cmd_buffer.handle(), buffer->handle(), (*image).handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    void Image::cmd_store(ReadyCommandBuffer cmd_buffer, Buffer *buffer, const std::vector<VkBufferImageCopy> &copy_regions)
    {
        if (copy_regions.size() > 0)
        {
            vkCmdCopyImageToBuffer(cmd_buffer.handle(), (*image).handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle(), static_cast<uint32_t>(copy_regions.size()), copy_regions.data());
            return;
        }

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {_extent.width, _extent.height, 1};

        vkCmdCopyImageToBuffer(cmd_buffer.handle(), (*image).handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle(), 1, &region);
    }

    Image ImageBuilder::build() const
    {
        Image image;

        const auto &context = *Context::get();

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

        if (vmaCreateImage(context.get_allocator(), &image_info, &allocation_info, &(*image.image).handle, &*image.allocation, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Image creation failed!");
        }

        image._extent = _extent;
        image._format = _format;
        image._aspect = _aspect;

        return image;
    }
}