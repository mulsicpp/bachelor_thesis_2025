#include <cstdio>
#include "vk/Context.h"
#include <stdexcept>
#include <cstdio>

int main(void) {
	try {
		vk::Instance instance{ "Test" };

		auto devices = instance.query_physical_devices();

		for (const auto& device : devices) {
			printf("device %p\n", device.handle());
		}
	}
	catch (const std::exception& e) {
		fprintf(stderr, "exception was thrown: %s\n", e.what());
		return -1;
	}
	return 0;
}