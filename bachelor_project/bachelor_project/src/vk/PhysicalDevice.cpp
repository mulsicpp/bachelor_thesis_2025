#include "PhysicalDevice.h"

namespace vk {
	PhysicalDevice::PhysicalDevice(VkPhysicalDevice physical_device_handle) {
		m_physical_device = physical_device_handle;
	}
}