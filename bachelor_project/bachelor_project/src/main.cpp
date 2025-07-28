#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"

#include "App.h"

#include "vk_core/CommandBuffer.h"
#include "vk_core/Handle.h"

#include "vk_resources/Buffer.h"
#include "vk_pipeline/Shader.h"

#include "scene/Scene.h"


void test_move_semantics();
void test_command_buffer();
void test_buffer_copy();
void test_buffer_with_staging();
void test_shaders();

int main(void) {
	try {
		utils::LibManager lib_manager{};

		App app{};
		app.run();
	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}

vk::Handle<int*> create_int_handle(int i) {
	vk::Handle<int*> handle;
	*handle = new int{ i };
	return handle;
}

void test_move_semantics() {

	vk::Handle<int*> h1 = create_int_handle(1);
	vk::Handle<int*> h2 = create_int_handle(2);
	vk::Handle<int*> h3 = std::move(h1);

	h2 = std::move(h3);
}

void test_command_buffer() {
	vk::CommandBuffer command_buffer = vk::CommandBufferBuilder(vk::QueueType::Transfer)
		.single_use(true)
		.build();

	auto recorder = [](vk::ReadyCommandBuffer cmd_buffer) {};

	command_buffer
		.record(recorder)
		.submit()
		.wait();
}

void test_buffer_copy() {
	const char data1[] = "Hi I'm buffer#1!!";
	const char data2[] = "Hi I'm buffer#2!!";

	const uint32_t size1 = sizeof(data1) / sizeof(char);
	const uint32_t size2 = sizeof(data2) / sizeof(char);

	vk::Buffer b1 = vk::BufferBuilder()
		.staging_buffer()
		.size(size1)
		.data((void*)data1)
		.build();

	vk::Buffer b2 = vk::BufferBuilder()
		.staging_buffer()
		.size(size2)
		.data((void*)data2)
		.add_usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		.build();

	char* mapped_data1 = b1.mapped_data<char>();
	char* mapped_data2 = b2.mapped_data<char>();

	printf("mapped_data1: %s\n", mapped_data1);
	printf("mapped_data2: %s\n", mapped_data2);

	b1.copy_into(&b2);

	printf("copied buffer#1 to buffer#2\n");
	printf("mapped_data1: %s\n", mapped_data1);
	printf("mapped_data2: %s\n", mapped_data2);
}

void test_buffer_with_staging() {
	vk::Buffer b = vk::BufferBuilder()
		.size(6)
		.data((void*)"hello")
		.memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
		.usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
		.add_queue_type(vk::QueueType::Transfer)
		.use_mapping(false)
		.build();
}

void test_shaders() {
	vk::Shader vertex_shader = vk::ShaderBuilder()
		.vertex()
		.load_spirv("assets/shaders/vert.spv")
		.build();
	dbg_log("loaded vertex shader");

	vk::Shader fragment_shader = vk::ShaderBuilder()
		.fragment()
		.load_spirv("assets/shaders/frag.spv")
		.build();
	dbg_log("loaded fragment shader");

	try {
		vk::Shader invlaid_sahder = vk::ShaderBuilder()
			.fragment()
			.load_spirv("assets/shaders/bla.spv")
			.build();
	}
	catch (const std::runtime_error& e) {
		dbg_log("expected error: %s", e.what());
	}
}