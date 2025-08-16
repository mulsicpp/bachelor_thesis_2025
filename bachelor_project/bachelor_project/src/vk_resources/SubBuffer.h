#pragma once

#include "Buffer.h"
#include "vk_core/CommandBuffer.h"

namespace vk {
	class SubBuffer : public ptr::ToShared<SubBuffer> {
	private:
		ptr::Shared<Buffer> _buffer{};
		VkDeviceSize _offset{ 0 };
		VkDeviceSize _length{ VK_WHOLE_SIZE };
		VkDeviceSize _stride{ 0 };

	public:
		inline const ptr::Shared<Buffer>& buffer() const { return _buffer; }
		inline void set_buffer(const ptr::Shared<Buffer>& buffer) { _buffer = buffer; }
		inline VkDeviceSize offset() const { return _offset; }
		inline VkDeviceSize length() const { return _length == VK_WHOLE_SIZE ? _buffer->size() : _length; }
		inline VkDeviceSize stride() const { return _stride; }

		static SubBuffer from(Buffer&& buffer, VkDeviceSize offset = 0, VkDeviceSize length = VK_WHOLE_SIZE, VkDeviceSize stride = 0);
		static SubBuffer from(const ptr::Shared<Buffer>& buffer, VkDeviceSize offset = 0, VkDeviceSize length = VK_WHOLE_SIZE, VkDeviceSize stride = 0);
	};
}