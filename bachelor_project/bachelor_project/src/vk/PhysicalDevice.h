#pragma once

#include <vulkan/vulkan.h>

namespace vk {
	class PhysicalDevice {
	private:
		VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
	public:
		PhysicalDevice() = default;
		PhysicalDevice(VkPhysicalDevice physical_device_handle);

		inline VkPhysicalDevice handle() const {
			return m_physical_device;
		}
	};
}