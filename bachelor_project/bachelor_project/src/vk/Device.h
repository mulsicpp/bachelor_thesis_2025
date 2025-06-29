#pragma once
#include <vulkan/vulkan.h>
#include <vector>

#include "../utils/NoCopy.h"

#include "Instance.h"
#include "PhysicalDevice.h"

namespace vk {

	class Device : public ::utils::NoCopy, public Handle<VkDevice> {
	private:
		VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
		VkQueue m_graphics_queue = VK_NULL_HANDLE;
		VkQueue m_present_queue = VK_NULL_HANDLE;
	public:
		Device() = default;
		Device(Instance* p_instance, PhysicalDevice* p_physical_device);
		~Device();
	};
}