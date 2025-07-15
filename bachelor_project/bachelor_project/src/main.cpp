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

	MOVE_SEMANTICS_VK_HANDLE(Test, val)
};

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