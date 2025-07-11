#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"
#include "App.h"

int main(void) {
	try {

		utils::LibManager lib_manager;
		App app{};

		app.run();
	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}