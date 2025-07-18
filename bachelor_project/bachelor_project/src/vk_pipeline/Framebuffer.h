#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"

namespace vk {

	class RenderPass;

	class Framebuffer : public utils::Move, public ptr::ToShared<Framebuffer> {
		friend class RenderPass;
	private:
		Handle<VkFramebuffer> framebuffer{};

	public:
		Framebuffer() = default;

		inline VkFramebuffer handle() const { return *framebuffer; }
	};
}