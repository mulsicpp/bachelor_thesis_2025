#pragma once

#include "external/volk.h"

namespace vk {
	template<class T>
	VkFormat format_of_type();

	VkFormat find_depth_format();
}