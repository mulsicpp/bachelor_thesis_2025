#pragma once

#include "external/vk_mem_alloc.h"
#include "external/VkBootstrap.h"

namespace vk {
	class Allocator {
	private:
		VmaAllocator allocator;
	public:
		static Allocator create(const vkb::Instance& instance, const vkb::PhysicalDevice& physical_device, const vkb::Device& device);
		static void destroy(const Allocator& allocator);
	};
}