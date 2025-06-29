#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "../utils/NoCopy.h"
#include "PhysicalDevice.h"
#include "Handle.h"

namespace vk {

	class Instance : public ::utils::NoCopy, public Handle<VkInstance>
	{
	private:
		VkDebugUtilsMessengerEXT m_debug_messanger = VK_NULL_HANDLE;

	public:
		Instance() = default;

		Instance(const char* app_name);
		~Instance();

		std::vector<PhysicalDevice> query_physical_devices() const;

	private:
		static std::vector<const char*> get_extensions();
		void create_debug_messanger();
	};
}

