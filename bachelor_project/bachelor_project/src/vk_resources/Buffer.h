#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

namespace vk {
	enum class BufferType {
		Vertex,
		Index,
		Uniform,
		Staging
	};

	struct BufferInfo {
		uint32_t size;
		BufferType type;
		void* data;
	};

	class Buffer {
	public:
		VkBuffer buffer;
		VmaAllocation allocation;

	public:
		Buffer();

	private:
		void destroy();

		inline void mark_mowed() {
			buffer = VK_NULL_HANDLE;
		}

		inline bool was_mowed() {
			return buffer == VK_NULL_HANDLE;
		}
	};

}