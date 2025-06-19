#pragma once

#include "ext_macros.h"
#include <vulkan/vulkan.h>

namespace debug_utils {
	DECL_EXT_FN(vkDestroyDebugUtilsMessengerEXT);
	DECL_EXT_FN(vkCreateDebugUtilsMessengerEXT);

	static void load(VkInstance instance) {
		LD_EXT_FN_INS(vkDestroyDebugUtilsMessengerEXT);
		LD_EXT_FN_INS(vkCreateDebugUtilsMessengerEXT);
	}
}