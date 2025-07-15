#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

#include "utils/move.h"

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

		MOVE_SEMANTICS_VK_HANDLE(Buffer, buffer)
	};

}