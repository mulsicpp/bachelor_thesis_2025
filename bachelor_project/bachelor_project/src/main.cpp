#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"

#include "App.h"

#include "vk_core/CommandBuffer.h"

#include "vk_resources/Buffer.h"

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
void test_buffer();

int main(void) {
	try {

		utils::LibManager lib_manager;

		App app{};

		test_buffer();

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

void test_buffer() {
	const char data1[] = "Hi I'm buffer#1!!";
	const char data2[] = "Hi I'm buffer#2!!";

	const uint32_t size1 = sizeof(data1) / sizeof(char);
	const uint32_t size2 = sizeof(data2) / sizeof(char);

	vk::Buffer b1 = vk::BufferBuilder()
		.as_staging_buffer()
		.from_data(data1, size1)
		.build();

	vk::Buffer b2 = vk::BufferBuilder()
		.as_staging_buffer()
		.from_data(data2, size2)
		.add_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		.build();

	char* mapped_data1 = b1.get_mapped_data<char>();
	char* mapped_data2 = b2.get_mapped_data<char>();

	printf("mapped_data1: %s\n", mapped_data1);
	printf("mapped_data2: %s\n", mapped_data2);

	b1.copy_into(&b2);

	printf("copied buffer#1 to buffer#2\n");
	printf("mapped_data1: %s\n", mapped_data1);
	printf("mapped_data2: %s\n", mapped_data2);
}