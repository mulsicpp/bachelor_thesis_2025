#include "Buffer.h"

#include "vk_core/Context.h"

namespace vk {
	Buffer::Buffer()
		: buffer{VK_NULL_HANDLE}
		, allocation{VK_NULL_HANDLE}
	{}

	Buffer::~Buffer() {
		if (buffer == VK_NULL_HANDLE) return;

		vmaDestroyBuffer(Context::get()->allocator, buffer, allocation);
	}
}