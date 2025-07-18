#include "Buffer.h"

#include "vk_core/Context.h"

namespace vk {

	void Buffer::cmd_copy_into(ReadyCommandBuffer cmd_buf, Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions) {
		if (copy_regions.size() > 0) {
			vkCmdCopyBuffer(cmd_buf.handle(), this->buffer.get(), dst_buffer->buffer.get(), copy_regions.size(), copy_regions.data());
			return;
		}

		VkBufferCopy copy_region{};
		copy_region.size = dst_buffer->size < size ? dst_buffer->size : size;
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;

		vkCmdCopyBuffer(cmd_buf.handle(), this->buffer.get(), dst_buffer->buffer.get(), 1, &copy_region);
	}

	void Buffer::copy_into(Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions) {
		CommandBuffer command_buffer = CommandBufferBuilder(QueueType::Transfer)
			.single_use(true)
			.build();

		command_buffer
			.record([&](ReadyCommandBuffer cmd_buf) { this->cmd_copy_into(cmd_buf, dst_buffer, copy_regions); })
			.submit()
			.wait();
	}



	BufferBuilder::BufferBuilder()
		: _size{ 0 }
		, _data{ nullptr }
		, _usage{ 0 }
		, _memory_usage{ VMA_MEMORY_USAGE_UNKNOWN }
		, _queue_types{}
		, _use_mapping{ true }
	{}


	BufferBuilder::Ref BufferBuilder::staging_buffer() {
		_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		_memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
		_queue_types = { QueueType::Transfer };
		_use_mapping = true;

		return *this;
	}


	Buffer BufferBuilder::build() {
		Buffer buffer{};

		const auto& context = *Context::get();

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = _size;
		buffer_info.usage = _usage;

		if (_queue_types.size() == 0) {
			throw std::runtime_error("Buffer creation failed! No queue types specified");
		}

		const auto families = context.get_command_manager().get_required_families(_queue_types);
		buffer_info.sharingMode = families.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		buffer_info.queueFamilyIndexCount = families.size();
		buffer_info.pQueueFamilyIndices = families.data();


		VmaAllocationCreateInfo allocation_info{};
		allocation_info.usage = _memory_usage;
		allocation_info.flags = _use_mapping ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

		VmaAllocationInfo alloc_info{};
		if (vmaCreateBuffer(context.get_allocator(), &buffer_info, &allocation_info, &*buffer.buffer, &*buffer.allocation, &alloc_info) != VK_SUCCESS) {
			throw std::runtime_error("Buffer creation failed!");
		}

		buffer.size = _size;
		VkMemoryPropertyFlags props;
		vmaGetMemoryTypeProperties(context.get_allocator(), alloc_info.memoryType, &props);

		buffer.host_coherent = (props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;

		buffer.mapped_data = alloc_info.pMappedData;

		if (_data == nullptr)
			return buffer;

		if (buffer.mapped_data != nullptr) {
			memcpy(buffer.mapped_data, _data, _size);
		}
		else {
			if ((_usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) != 0) {
				Buffer staging_buffer = BufferBuilder()
					.staging_buffer()
					.size(_size)
					.data(_data)
					.build();

				staging_buffer.copy_into(&buffer);
			}
		}

		return buffer;
	}
}