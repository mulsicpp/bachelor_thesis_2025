#include "Fence.h"
#include "vk_core/Context.h"

namespace vk {
	Fence Fence::create(VkFenceCreateFlags flags) {
		Fence fence;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = flags;

		if (vkCreateFence(Context::get()->get_device(), &fence_info, nullptr, &*fence.fence) != VK_SUCCESS) {
			throw std::runtime_error("Fence creation failed!");
		}

		return fence;
	}

	void Fence::reset() {
		vkResetFences(Context::get()->get_device(), 1, &*fence);
	}

	void Fence::wait(uint64_t timeout) {
		vkWaitForFences(Context::get()->get_device(), 1, &*fence, VK_TRUE, timeout);
	}
}