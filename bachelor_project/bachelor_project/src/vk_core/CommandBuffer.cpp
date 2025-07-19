#include "CommandBuffer.h"
#include "Context.h"

// #define DEBUG_HANDLE
#ifdef DEBUG_HANDLE
#include "utils/dbg_log.h"
#include <typeinfo>
#define HANDLE_LOG(msg) { dbg_log("%s %p %s", typeid(VkCommandBuffer).name(), command_buffer, msg); }
#else
#define HANDLE_LOG(msg) 
#endif

namespace vk {

	CommandBuffer::~CommandBuffer() {
		if (command_buffer != VK_NULL_HANDLE) {
			HANDLE_LOG("destroyed");
			destroy();
		}
	}

	CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept 
		: command_buffer{ other.command_buffer }
		, queue{ other.queue }
		, fence{ std::move(other.fence) }
		, is_single_use { other.is_single_use }
		, used { other.used }
	{
		other.command_buffer = VK_NULL_HANDLE;
		HANDLE_LOG("moved in constructor");
	}
	CommandBuffer::Ref CommandBuffer::operator=(CommandBuffer&& other) noexcept {
		if (this == &other) return *this;

		if (command_buffer != VK_NULL_HANDLE) {
			destroy();
			HANDLE_LOG("destroyed");
		}
		command_buffer = other.command_buffer;
		queue = other.queue;
		fence = std::move(other.fence);
		is_single_use = other.is_single_use;
		used = other.used;

		other.command_buffer = VK_NULL_HANDLE;

		HANDLE_LOG("moved in assignment");

		return *this;
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

		std::vector<VkSemaphore> wait_semaphores{};
		std::vector<VkSemaphore> signal_semaphores{};

		for (const auto& s : info.wait_semaphores) {
			wait_semaphores.push_back(s.handle());
		}

		for (const auto& s : info.signal_semaphores) {
			signal_semaphores.push_back(s.handle());
		}

		submit_info.waitSemaphoreCount = wait_semaphores.size();
		submit_info.pWaitSemaphores = wait_semaphores.data();
		submit_info.pWaitDstStageMask = info.wait_dst_stage_masks.data();

		submit_info.signalSemaphoreCount = signal_semaphores.size();
		submit_info.pSignalSemaphores = signal_semaphores.data();

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		fence.reset();

		if (vkQueueSubmit(queue.queue, 1, &submit_info, fence.handle()) != VK_SUCCESS) {
			throw std::runtime_error("Command buffer submission failed!");
		}

		if (is_single_use)
			used = true;

		return *this;
	}

	CommandBuffer::Ref CommandBuffer::wait() {
		fence.wait();
		return *this;
	}



	void CommandBuffer::destroy() {
		const auto& context = *Context::get();

		wait();
		vkFreeCommandBuffers(context.get_device(), queue.command_pool, 1, &command_buffer);
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

		cmd_buffer.fence = Fence::create(VK_FENCE_CREATE_SIGNALED_BIT);

		cmd_buffer.is_single_use = _single_use;
		cmd_buffer.used = false;

		return cmd_buffer;
	}
}

#undef HANDLE_LOG