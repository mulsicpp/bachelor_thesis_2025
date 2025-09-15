#pragma once

#include "external/volk.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

namespace vk {
	class Fence : public utils::Move, public ptr::ToShared<Fence> {
	private:
		Handle<VkFence> fence{};

	public:
		Fence() = default;

		static Fence create(VkFenceCreateFlags flags);

		inline VkFence handle() const { return *fence; }

		void reset();
		void wait(uint64_t timeout = UINT64_MAX);
	};
}