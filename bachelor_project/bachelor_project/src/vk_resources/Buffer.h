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

	struct BufferBuilder {
		uint32_t size;
		BufferType type;
		void* data;


	};

	class Buffer {
		friend class BufferBuidler;
	private:
		VkBuffer buffer;
		VmaAllocation allocation;
	public:
		Buffer();
		~Buffer();
	private:
	};
}