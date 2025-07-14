#include "CommandManager.h"

#include "utils/dbg_log.h"

#include <map>
#include <set>
#include <string>

static const std::array<vkb::QueueType, QUEUE_TYPE_COUNT> QUEUE_TYPES = {
	vkb::QueueType::present,
	vkb::QueueType::graphics,
	vkb::QueueType::compute,
	vkb::QueueType::transfer
};

static const std::array<const char*, QUEUE_TYPE_COUNT> QUEUE_TYPE_NAMES = {
	"Present",
	"Graphics",
	"Compute",
	"Transfer"
};

namespace vk {

	QueueInfo QueueInfo::get_from_physical_device(vkb::PhysicalDevice device) {
		QueueInfo queue_info;

		auto queue_families = device.get_queue_families();
		queue_info.queue_family_count = queue_families.size();

		for (auto& idx : queue_info.queue_family_indices) {
			idx = -1;
		}

		VkBool32 present_support = false;

		for (uint32_t i = 0; i < queue_families.size(); i++) {

			vkGetPhysicalDeviceSurfaceSupportKHR(device.physical_device, i, device.surface, &present_support);
			if (present_support) {
				queue_info.queue_family_indices[QueueType::Present] = i;
				queue_info.used_families.insert(i);
				break;
			}
		}

		for (uint32_t i = 0; i < queue_families.size(); i++) {

			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queue_info.queue_family_indices[QueueType::Graphics] = i;
				queue_info.used_families.insert(i);
				break;
			}
		}

		for (uint32_t i = 0; i < queue_families.size(); i++) {

			if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
				queue_info.queue_family_indices[QueueType::Compute] = i;
				queue_info.used_families.insert(i);
				break;
			}
		}

		for (uint32_t i = 0; i < queue_families.size(); i++) {

			if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
				queue_info.queue_family_indices[QueueType::Transfer] = i;
				queue_info.used_families.insert(i);
				break;
			}
		}

		for (int i = 0; i < queue_info.queue_family_indices.size(); i++) {
			if (queue_info.queue_family_indices[i] < 0) {
				throw std::runtime_error(std::string("No queue family found for queue type '") + QUEUE_TYPE_NAMES[i] + "'");
			}
		}

		return queue_info;
	}

	std::vector<vkb::CustomQueueDescription> QueueInfo::get_custom_queue_descriptions() {
		std::vector<vkb::CustomQueueDescription> custom_queue_descriptions;
		for (auto family_index : used_families) {
			custom_queue_descriptions.push_back(vkb::CustomQueueDescription{ family_index, 1, {1.0} });
		}

		return custom_queue_descriptions;
	}

	VkCommandPool create_command_pool(VkDevice device, uint32_t family_index) {
		VkCommandPool command_pool;

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = family_index;

		if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
			throw std::runtime_error("Command pool creation failed for family index '" + std::to_string(family_index) + "'");
		}

		return command_pool;
	}

	CommandManager CommandManager::create(const vkb::Device& device, const QueueInfo& queue_info) {
		CommandManager manager;

		manager.device = device.device;
		manager.command_pools = std::vector<VkCommandPool>(queue_info.queue_family_count);

		for (auto& pool : manager.command_pools) {
			pool = VK_NULL_HANDLE;
		}

		for (const auto idx : queue_info.used_families) {
			manager.command_pools[idx] = create_command_pool(device.device, idx);
		}

		for (int i = 0; i < QUEUE_TYPE_COUNT; i++) {
			vkGetDeviceQueue(device.device, queue_info.queue_family_indices[i], 0, &manager.queues[i].queue);
			manager.queues[i].family_index = queue_info.queue_family_indices[i];
		}

		for (const auto command_pool : manager.command_pools) {
			dbg_log("pool: %p", command_pool);
		}

		for (const auto queue : manager.queues) {
			dbg_log("queue: %p pool_index: %i", queue.queue, queue.family_index);
		}

		return manager;
	}

	void CommandManager::destroy(const vkb::Device& device, const CommandManager& manager) {
		for (const auto command_pool : manager.command_pools) {
			if(command_pool != VK_NULL_HANDLE)
				vkDestroyCommandPool(device.device, command_pool, nullptr);
		}

	}
}