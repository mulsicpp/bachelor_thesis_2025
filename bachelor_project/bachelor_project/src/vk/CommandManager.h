#pragma once

#include "external/VkBootstrap.h"
#include "utils/NoCopy.h"

#include <array>
#include <set>

#define QUEUE_TYPE_COUNT 4

namespace vk {

	enum QueueType {
		Present = 0,
		Graphics = 1,
		Compute = 2,
		Transfer = 3
	};

	struct QueueInfo {
		uint32_t queue_family_count;
		std::set<uint32_t> used_families;
		std::array<int, QUEUE_TYPE_COUNT> queue_family_indices;

		static QueueInfo get_from_physical_device(vkb::PhysicalDevice device);

		std::vector<vkb::CustomQueueDescription> get_custom_queue_descriptions();
	};

	struct Queue {
		VkQueue queue = VK_NULL_HANDLE;
		uint32_t pool_index = -1;
	};

	class CommandManager {
	private:
		VkDevice m_device = VK_NULL_HANDLE;
		std::array<Queue, QUEUE_TYPE_COUNT> m_queues;
		std::vector<VkCommandPool> m_command_pools;
	public:
		static CommandManager create(vkb::Device device, QueueInfo queue_info);
		static void destroy(CommandManager& manager);
	};
}