#include "Buffer.h"

#include "vk_core/Context.h"

namespace vk {
	Buffer::Buffer()
		: buffer{VK_NULL_HANDLE}
		, allocation{VK_NULL_HANDLE}
	{}

	void Buffer::destroy() {
		vmaDestroyBuffer(Context::get()->get_allocator(), buffer, allocation);
	}
}