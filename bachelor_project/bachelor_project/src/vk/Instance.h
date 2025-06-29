#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "../utils/NoCopy.h"
#include "PhysicalDevice.h"

namespace vk {

	class Instance : ::utils::NoCopy
	{
	private:
		VkInstance m_instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debug_messanger = VK_NULL_HANDLE;

	public:
		Instance() = default;

		Instance(const char* app_name);
		~Instance();

		inline VkInstance handle() const {
			return m_instance;
		}

		std::vector<PhysicalDevice> query_physical_devices() const;

	private:
		static std::vector<const char*> get_extensions();
		void create_debug_messanger();
	};
}

