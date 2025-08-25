#include "Image.h"
#include "vk_core/Context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace vk
{
#define FORMAT_CASES(comp)         \
    case VK_FORMAT_##comp##_UINT:  \
    case VK_FORMAT_##comp##_UNORM: \
    case VK_FORMAT_##comp##_SNORM: \
    case VK_FORMAT_##comp##_USCALED

    static int get_channel_count(VkFormat format)
    {
        switch (format)
        {
            FORMAT_CASES(R8) : return 1;
            FORMAT_CASES(R8G8B8) : return 3;
            FORMAT_CASES(R8G8B8A8) : return 4;
        }

        return -1;
    }

    struct ImageStateInfo
    {
        VkImageLayout layout;
        VkAccessFlags access_mask;
        VkPipelineStageFlags stage_mask;
    };

    static const std::vector<ImageStateInfo> IMAGE_STATE_INFOS = {
        {VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT},
        {VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
        {VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT},
        {VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}};

    void Image::cmd_transition(ReadyCommandBuffer cmd_buffer, ImageState src_state, ImageState dst_state)
    {
        const auto &src_info = IMAGE_STATE_INFOS[(uint32_t)src_state];
        const auto &dst_info = IMAGE_STATE_INFOS[(uint32_t)dst_state];

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

    void Image::transition(ImageState src_state, ImageState dst_state)
    {
        CommandBuffer::single_time_submit(QueueType::Transfer, [&](ReadyCommandBuffer cmd_buffer)
                                          { cmd_transition(cmd_buffer, src_state, dst_state); });
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

    void Image::store_in_file(const std::string &file)
    {
        int channel_count = get_channel_count(_format);
        if (channel_count == -1)
        {
            throw std::runtime_error("Could not store image in file '" + file + "'! Format not compatible");
        }

        Buffer buffer = BufferBuilder()
                            .usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                            .add_queue_type(QueueType::Transfer)
                            .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
                            .size(_extent.width * _extent.height * channel_count)
                            .build();

        CommandBuffer::single_time_submit(QueueType::Transfer, [&](ReadyCommandBuffer cmd_buffer)
                                          { cmd_store(cmd_buffer, &buffer); });

        auto *data = buffer.mapped_data<uint8_t>();

        stbi_write_png(file.c_str(), _extent.width, _extent.height, channel_count, buffer.mapped_data<void>(), 0);
    }

    Image ImageBuilder::build() const
    {
        Image image;

        const auto &context = *Context::get();

        int width{_extent.width}, height{_extent.height}, comp{};
        const uint8_t *pixels{nullptr};

        int required_channel_count = get_channel_count(_format);

        if (!_file.empty())
        {
            if (required_channel_count == -1)
            {
                throw std::runtime_error("Image creation failed! Format not compatible");
            }
            pixels = stbi_load(_file.c_str(), &width, &height, &comp, required_channel_count);
        }

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.format = _format;
        image_info.tiling = _tiling;
        image_info.initialLayout = pixels != nullptr ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
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

        if (pixels != nullptr)
        {
            Buffer staging_buffer = BufferBuilder()
                                        .usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
                                        .add_queue_type(QueueType::Transfer)
                                        .memory_usage(VMA_MEMORY_USAGE_CPU_COPY)
                                        .size(width * height * required_channel_count)
                                        .data((void *)pixels)
                                        .build();

            CommandBuffer::single_time_submit(QueueType::Transfer, [&](ReadyCommandBuffer cmd_buffer)
                                              { image.cmd_load(cmd_buffer, &staging_buffer); });
        }

        image._extent = _extent;
        image._format = _format;
        image._aspect = _aspect;

        return image;
    }
}