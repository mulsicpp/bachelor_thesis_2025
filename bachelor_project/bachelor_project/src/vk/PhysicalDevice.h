#pragma once

#include <vulkan/vulkan.h>

#include "Handle.h"

namespace vk {
	class PhysicalDevice : public Handle<VkPhysicalDevice> {
	public:
		PhysicalDevice() = default;
		PhysicalDevice(VkPhysicalDevice physical_device_handle);
	};
}