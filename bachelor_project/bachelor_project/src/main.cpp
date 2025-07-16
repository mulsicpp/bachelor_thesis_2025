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

	MOVE_SEMANTICS_VK_DEFAULT(Test, val)
};

void test_move_semantics();

void test_command_buffer();

int main(void) {
	try {

		utils::LibManager lib_manager;

		App app{};

		test_command_buffer();

		app.run();

		// test_move_semantics();
	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}

void test_move_semantics() {
	Test t1 = Test::create(1);
	Test t2 = Test::create(2);

	Test t3 = std::move(t2);

	t1 = std::move(t3);
}

void test_command_buffer() {
	vk::CommandBuffer command_buffer = vk::CommandBufferBuilder(vk::QueueType::Transfer)
		.set_single_use(true)
		.build();

	auto recorder = [](vk::ReadyCommandBuffer cmd_buffer) {};

	command_buffer
		.record(recorder)
		.submit()
		.wait();
}