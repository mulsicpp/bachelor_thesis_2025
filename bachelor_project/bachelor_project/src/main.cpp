#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"

#include "App.h"

#include "vk_core/CommandBuffer.h"

int main(void) {
	try {

		utils::LibManager lib_manager;

		App app{};

		{
			auto command_buffer = vk::CommandBufferBuilder(vk::QueueType::Graphics).build();

			auto recorder = [](VkCommandBuffer commandbuffer) {};

			command_buffer
				.record(recorder)
				.submit()
				.wait();

			dbg_log("command buffer finished");
		}

		app.run();

	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}