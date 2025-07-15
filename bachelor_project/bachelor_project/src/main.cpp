#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"

#include "App.h"

#include "vk_core/CommandBuffer.h"

class Test {
public:
	int* val{};

	Test() = default;

	static Test create(int val) { 
		dbg_log("created %i", val);
		Test test;
		test.val = new int{val};
		return test;
	}

private:
	void destroy() { dbg_log("destroyed %i", *val); delete val; }
	void mark_moved() { dbg_log("moved %i", val ? *val : -1); val = nullptr; }
	bool was_moved() { return val == nullptr; }

	MOVE_SEMANTICS(Test)
};

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

		/*Test t1 = Test::create(1);

		Test t2 = Test::create(2);

		t1 = std::move(t2);

		t1 = std::move(t2);*/

	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}