#include "Semaphore.h"
#include "vk_core/Context.h"

namespace vk {
	Semaphore Semaphore::create() {
		Semaphore semaphore;

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(Context::get()->get_device(), &semaphore_info, nullptr, &*semaphore.semaphore) != VK_SUCCESS) {
			throw std::runtime_error("Semaphore creation failed!");
		}

		return semaphore;
	}
}