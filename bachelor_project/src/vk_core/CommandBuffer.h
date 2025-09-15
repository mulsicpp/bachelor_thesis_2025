#pragma once

#include "CommandManager.h"

#include "utils/move.h"
#include "utils/ptr.h"

#include "vk_sync/Fence.h"
#include "vk_sync/Semaphore.h"

#include <functional>

namespace vk {

	class ReadyCommandBuffer;
	using CommandRecorder = std::function<void(ReadyCommandBuffer)>;

	struct SubmitInfo : public utils::Move {
		using Ref = SubmitInfo&;
		std::vector<Semaphore> signal_semaphores{};
		std::vector<Semaphore> wait_semaphores{};
		std::vector<VkPipelineStageFlags> wait_dst_stage_masks{};

		inline Ref add_signal_semaphore(Semaphore&& semaphore) {
			signal_semaphores.emplace_back(std::move(semaphore));
			return *this;
		}

		inline Ref add_wait_semaphore(Semaphore&& semaphore, VkPipelineStageFlags dst_stage_mask) {
			wait_semaphores.emplace_back(std::move(semaphore));
			wait_dst_stage_masks.push_back(dst_stage_mask);
			return *this;
		}
	};

	class CommandBufferBuilder;

	class CommandBuffer : public ptr::ToShared<CommandBuffer> {
		friend class CommandBufferBuilder;
	public:
		using Ref = CommandBuffer&;
	private:
		VkCommandBuffer command_buffer{ VK_NULL_HANDLE };
		Queue queue{};
		Fence fence{};

		bool is_single_use{ false };
		bool used{ false };

	public:
		CommandBuffer() = default;
		~CommandBuffer();

		CommandBuffer(CommandBuffer&& other) noexcept;
		Ref operator=(CommandBuffer&& other) noexcept;

		CommandBuffer(const CommandBuffer& other) = delete;
		Ref operator=(const CommandBuffer& other) = delete;

		Ref record(CommandRecorder recorder);
		Ref submit(const SubmitInfo& info = {});

		Ref wait();

		static void single_time_submit(QueueType type, CommandRecorder recorder);
	private:
		void destroy();
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