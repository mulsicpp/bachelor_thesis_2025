#include "Allocator.h"

namespace vk {
	Allocator Allocator::create(const vkb::Instance& instance, const vkb::PhysicalDevice& physical_device, const vkb::Device& device) {
		Allocator allocator;

		VmaAllocatorCreateInfo allocator_info;
		allocator_info.instance = instance.instance;
		allocator_info.physicalDevice = physical_device.physical_device;
		allocator_info.device = device.device;

		allocator_info.frameInUseCount = 0;

		allocator_info.vulkanApiVersion = 0;
		allocator_info.flags = 0;
		allocator_info.preferredLargeHeapBlockSize = 0;

		allocator_info.pAllocationCallbacks = nullptr;
		allocator_info.pDeviceMemoryCallbacks = nullptr;
		allocator_info.pHeapSizeLimit = nullptr;
		allocator_info.pVulkanFunctions = nullptr;
		allocator_info.pRecordSettings = nullptr;

		vmaCreateAllocator(&allocator_info, &allocator.allocator);

		return allocator;
	}

	void Allocator::destroy(const Allocator& allocator) {
		vmaDestroyAllocator(allocator.allocator);
	}
}