#pragma once

#include "CommandManager.h"

#include "utils/move.h"

namespace vk {

	class CommandBufferBuilder;

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

	class CommandBufferBuilder {
	public:
		using Ref = CommandBufferBuilder&;

		QueueType queue_type;
		bool single_use;
	public:
		CommandBufferBuilder(QueueType queue_type);

		CommandBuffer build();

		inline Ref set_single_use(bool is_single_use) {
			this->single_use = is_single_use;
			return *this;
		}
	};
}