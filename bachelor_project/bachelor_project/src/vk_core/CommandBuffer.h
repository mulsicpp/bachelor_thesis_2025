#pragma once

#include "CommandManager.h"

#include "utils/move.h"

#include <functional>

namespace vk {

	class ReadyCommandBuffer;
	using CommandRecorder = std::function<void(ReadyCommandBuffer)>;

	struct SubmitInfo {
		using Ref = SubmitInfo&;
		std::vector<VkSemaphore> signal_semaphores{};
		std::vector<VkSemaphore> wait_semaphores{};
		std::vector<VkPipelineStageFlags> wait_dst_stage_masks{};

		inline Ref add_signal_semaphore(VkSemaphore semaphore) {
			signal_semaphores.push_back(semaphore);
			return *this;
		}

		inline Ref add_wait_semaphore(VkSemaphore semaphore, VkPipelineStageFlags dst_stage_mask) {
			wait_semaphores.push_back(semaphore);
			wait_dst_stage_masks.push_back(dst_stage_mask);
			return *this;
		}
	};

	class CommandBufferBuilder;

	class CommandBuffer {
		friend class CommandBufferBuilder;
	public:
		using Ref = CommandBuffer&;
	private:
		VkCommandBuffer command_buffer;
		Queue queue;
		VkFence fence;

		bool is_single_use;
		bool used;

	public:
		CommandBuffer();

		Ref record(CommandRecorder recorder);
		Ref submit(const SubmitInfo& info = {});

		Ref wait();
	private:
		void destroy();

		MOVE_SEMANTICS_VK_DEFAULT(CommandBuffer, command_buffer)
	};

	class ReadyCommandBuffer {
		friend class CommandBuffer;
	private:
		VkCommandBuffer command_buffer;

		inline ReadyCommandBuffer(VkCommandBuffer handle) : command_buffer{handle} {}

	public:
		inline VkCommandBuffer handle() { return command_buffer; }
	};

	class CommandBufferBuilder {
	public:
		using Ref = CommandBufferBuilder&;

	private:
		QueueType _queue_type;
		bool _single_use;

	public:
		CommandBufferBuilder(QueueType queue_type);

		inline Ref single_use(bool single_use) {
			_single_use = single_use;
			return *this;
		}


		CommandBuffer build();
	};
}