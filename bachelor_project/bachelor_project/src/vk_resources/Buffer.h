#pragma once

#include <vulkan/vulkan.h>
#include "external/vk_mem_alloc.h"

#include "utils/move.h"

#include "vk_core/CommandBuffer.h"

#include <vector>

namespace vk {

	class BufferBuilder;
	
	class Buffer {
		friend class BufferBuilder;
	private:
		VkBuffer buffer;
		VmaAllocation allocation;

		uint32_t size;
		void* mapped_data;
		bool host_coherent;

	public:
		Buffer();

		inline VkBuffer handle() const { return buffer; }
		template<class T = uint8_t>
		inline T* get_mapped_data() { return (T*)mapped_data; }

		void cmd_copy_into(ReadyCommandBuffer cmd_buf, Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions = {});
		void copy_into(Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions = {});

	private:
		void destroy();

		MOVE_SEMANTICS_VK_DEFAULT(Buffer, buffer)
	};



	struct BufferBuilder {
		using Ref = BufferBuilder&;

		uint32_t size;
		void* data;
		VkBufferUsageFlagBits usage;
		VmaMemoryUsage memory_usage;
		std::vector<QueueType> queue_types;
		bool use_mapping;

		BufferBuilder();

		inline Ref set_size(uint32_t size) { this->size = size; return* this; }

		template<class T = uint8_t>
		inline Ref from_data(T* data, uint32_t size) {
			this->data = (void*)data;
			this->size = size * sizeof(T);
			return *this;
		}

		inline Ref set_usage(VkBufferUsageFlagBits usage) { this->usage = usage; return *this; }
		inline Ref add_usage(VkBufferUsageFlagBits usage) { this->usage = (VkBufferUsageFlagBits)(this->usage | usage); return *this; }

		Ref as_staging_buffer();

		Buffer build();
	};
}