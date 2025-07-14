#pragma once

#include "external/VkBootstrap.h"
#include "utils/NoCopy.h"

#include <array>
#include <set>
#include <functional>

#define QUEUE_TYPE_COUNT 4

namespace vk {

	using CommandRecorder = std::function<void(VkCommandBuffer)>;

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
		int32_t family_index = -1;
		VkCommandPool command_pool;
	};


	class CommandManager {
		friend class CommandBuffer;
	private:
		VkDevice device = VK_NULL_HANDLE;
		std::array<Queue, QUEUE_TYPE_COUNT> queues;
		std::vector<VkCommandPool> command_pools;
	public:
		static CommandManager create(const vkb::Device& device, const QueueInfo& queue_info);
		static void destroy(const vkb::Device& device, const CommandManager& manager);

		inline const Queue& operator[](QueueType type) const {
			return queues[(uint32_t)type];
		}

		inline Queue& operator[](QueueType type) {
			return queues[(uint32_t)type];
		}
	};
}