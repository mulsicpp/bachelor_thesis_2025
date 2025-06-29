#include "PhysicalDevice.h"

namespace vk {
	PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device_handle) {
		m_handle = physical_device_handle;
	}
}