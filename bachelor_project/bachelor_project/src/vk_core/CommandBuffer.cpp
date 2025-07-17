#include "CommandBuffer.h"
#include "Context.h"

namespace vk {

	CommandBuffer::CommandBuffer()
		: command_buffer{ VK_NULL_HANDLE }
		, queue{}
		, fence{ VK_NULL_HANDLE }
		, is_single_use{ false }
		, used{ false }
	{}

	void CommandBuffer::destroy() {
		const auto& context = *Context::get();

		wait();
		vkFreeCommandBuffers(context.get_device(), queue.command_pool, 1, &command_buffer);
		vkDestroyFence(context.get_device(), fence, nullptr);
	}

	CommandBuffer::Ref CommandBuffer::record(CommandRecorder recorder) {
		if (is_single_use && used) {
			throw std::runtime_error("Cannot rerecord single-use buffer!");
		}

		const auto& context = *Context::get();

		vkResetCommandBuffer(command_buffer, /*VkCommandBufferResetFlagBits*/ 0);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = is_single_use ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer recording failed at beginning!");
		}

		recorder(command_buffer);

		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer recording failed at end!");
		}

		return *this;
	}

	CommandBuffer::Ref CommandBuffer::submit(const SubmitInfo& info) {
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submit_info.waitSemaphoreCount = info.wait_semaphores.size();
		submit_info.pWaitSemaphores = info.wait_semaphores.data();
		submit_info.pWaitDstStageMask = info.wait_dst_stage_masks.data();

		submit_info.signalSemaphoreCount = info.signal_semaphores.size();
		submit_info.pSignalSemaphores = info.signal_semaphores.data();

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkResetFences(Context::get()->get_device(), 1, &fence);

		if (vkQueueSubmit(queue.queue, 1, &submit_info, fence) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer submission failed!");
		}

		if (is_single_use)
			used = true;

		return *this;
	}



	CommandBuffer::Ref CommandBuffer::wait() {
		vkWaitForFences(Context::get()->get_device(), 1, &fence, VK_TRUE, UINT64_MAX);

		return *this;
	}



	CommandBufferBuilder::CommandBufferBuilder(QueueType queue_type)
		: _queue_type{ queue_type }
		, _single_use{ false }
	{}

	CommandBuffer CommandBufferBuilder::build() {
		CommandBuffer cmd_buffer;

		const auto& context = *Context::get();

		auto command_manager = context.get_command_manager();

		cmd_buffer.queue = command_manager[_queue_type];

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = cmd_buffer.queue.command_pool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(context.get_device(), &alloc_info, &cmd_buffer.command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer allocation failed!");
		}

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(context.get_device(), &fence_info, nullptr, &cmd_buffer.fence) != VK_SUCCESS) {
			throw std::runtime_error("Fence creation failed!");
		}

		cmd_buffer.is_single_use = _single_use;
		cmd_buffer.used = false;

		return cmd_buffer;
	}
}