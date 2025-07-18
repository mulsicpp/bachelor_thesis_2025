#pragma once

#include <vulkan/vulkan.h>

#include "external/vk_mem_alloc.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_core/Handle.h"
#include "vk_core/CommandBuffer.h"

#include <vector>

namespace vk {

	class BufferBuilder;

	class Buffer : public utils::Move, public ptr::ToShared<Buffer> {
		friend class BufferBuilder;
	private:
		Handle<VmaAllocation> allocation{};
		Handle<VkBuffer> buffer{};

		uint32_t size{ 0 };
		void* mapped_data{ nullptr };
		bool host_coherent{ false };

	public:
		Buffer() = default;

		inline VkBuffer handle() const { return *buffer; }
		template<class T = uint8_t>
		inline T* get_mapped_data() { return (T*)mapped_data; }

		void cmd_copy_into(ReadyCommandBuffer cmd_buf, Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions = {});
		void copy_into(Buffer* dst_buffer, const std::vector<VkBufferCopy>& copy_regions = {});
	};



	class BufferBuilder {
	public:
		using Ref = BufferBuilder&;

	private:
		uint32_t _size;
		void* _data;
		VkBufferUsageFlags _usage;
		VmaMemoryUsage _memory_usage;
		std::vector<QueueType> _queue_types;
		bool _use_mapping;
	
	public:
		BufferBuilder();

		inline Ref size(uint32_t size) { _size = size; return *this; }
		inline Ref data(void* data) { _data = data; return *this; }

		inline Ref usage(VkBufferUsageFlags usage) { _usage = usage; return *this; }
		inline Ref add_usage(VkBufferUsageFlagBits usage) { _usage |= usage; return *this; }

		inline Ref memory_usage(VmaMemoryUsage memory_usage) { _memory_usage = memory_usage; return *this; }

		inline Ref queue_types(const std::vector<QueueType>& queue_types) { _queue_types = queue_types; return *this; }
		inline Ref add_queue_type(QueueType queue_type) { _queue_types.push_back(queue_type); return *this; }

		inline Ref use_mapping(bool use_mapping) { _use_mapping = use_mapping; return *this; }

		Ref staging_buffer();


		Buffer build();
	};
}