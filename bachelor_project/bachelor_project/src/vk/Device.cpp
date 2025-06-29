#include "Device.h"

namespace vk {
	Device::Device(Instance* p_instance, PhysicalDevice* p_physical_device) {

	}

	Device::~Device() {
		if (m_device != VK_NULL_HANDLE) {
			vkDestroyDevice(m_device, nullptr);
		}
	}
}