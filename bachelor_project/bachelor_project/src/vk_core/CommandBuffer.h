#pragma once

#include "CommandManager.h"

namespace vk {

	class CommandBufferBuilder;

	struct SubmitInfo {
		using Ref = SubmitInfo&;
		std::vector<VkSemaphore> signal_semaphores;
		std::vector<VkSemaphore> wait_semaphores;
		std::vector<VkPipelineStageFlags> wait_dst_stage_masks;

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
		~CommandBuffer();

		Ref record(CommandRecorder recorder);
		Ref submit(const SubmitInfo& info);

		Ref wait();
		Ref reset_fence();
	};


	enum class FenceState { Unsignaled, Signaled };

	class CommandBufferBuilder {
	public:
		using Ref = CommandBufferBuilder&;

		QueueType queue_type;
		bool is_single_use;
		bool has_fence;
		FenceState fence_start_state;
	public:
		CommandBufferBuilder(QueueType queue_type);

		CommandBuffer build();

		inline Ref with_single_use(bool is_single_use) {
			this->is_single_use = is_single_use;
			return *this;
		}

		inline Ref with_fence(FenceState start_state = FenceState::Unsignaled) {
			this->has_fence = true;
			this->fence_start_state = start_state;
			return *this;
		}

		inline Ref without_fence() {
			this->has_fence = false;
			return *this;
		}
	};
}