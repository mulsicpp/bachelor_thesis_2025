#pragma once

#include <vulkan/vulkan.h>

#include "utils/move.h"

namespace vk {

	class RenderPass {
	private:
		VkRenderPass render_pass;

	public:
		RenderPass();

	private:
		void destroy();

		MOVE_SEMANTICS_VK_HANDLE(RenderPass, render_pass)
	};
}