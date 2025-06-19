#pragma once

#include "../utils/NoCopy.h"
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace vk {

	class Instance : ::utils::NoCopy
	{
	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debug_messanger;

	public:
		Instance(const char* app_name);
		~Instance();

		inline VkInstance handle() const {
			return m_instance;
		}

	private:
		static std::vector<const char*> get_extensions();
		void create_debug_messanger();
	};
}

