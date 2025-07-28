#include "SubBuffer.h"

namespace vk {
	SubBuffer SubBuffer::from(Buffer&& buffer, VkDeviceSize offset, VkDeviceSize length, VkDeviceSize stride) {
		return from(std::move(buffer).to_shared(), offset, length, stride);
	}

	SubBuffer SubBuffer::from(const ptr::Shared<Buffer>& buffer, VkDeviceSize offset, VkDeviceSize length, VkDeviceSize stride) {
		SubBuffer sub_buffer{};

		sub_buffer._buffer = buffer;
		sub_buffer._offset = offset;
		sub_buffer._length = length;
		sub_buffer._stride = stride;

		return sub_buffer;
	}
}