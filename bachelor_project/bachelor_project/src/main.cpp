#include <cstdio>
#include "vk_context/Instance.h"
#include <stdexcept>
#include <cstdio>

int main(void) {
	try {
		vk::Instance instance("My Renderer");
	}
	catch (const std::exception& e) {
		fprintf(stderr, "exception was thrown: %s\n", e.what());
		return -1;
	}
	return 0;
}