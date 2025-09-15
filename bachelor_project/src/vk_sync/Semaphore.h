#pragma once

#include "external/volk.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

namespace vk {
	class Semaphore : public utils::Move, public ptr::ToShared<Semaphore> {
	private:
		Handle<VkSemaphore> semaphore{};

	public:
		Semaphore() = default;

		static Semaphore create();

		inline VkSemaphore handle() const { return *semaphore; }
	};
}
