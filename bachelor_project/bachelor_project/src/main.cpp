#include <cstdio>
#include <stdexcept>

#include "utils/LibManager.h"
#include "utils/move.h"

#include "App.h"

class Test {
public:
	int* val;

	Test() : val{ nullptr } {}

	Test(int i) : val{ new int(i) } {
		dbg_log("created %i", *val);
	}

	void destroy() {
		dbg_log("destroyed %i", *val);
		delete val;
	}

	void mark_moved() {
		dbg_log("moved %i", *val);
		val = nullptr;
	}

	bool was_moved() {
		return val == nullptr;
	}

	MOVE_SEMANTICS(Test)
};

int main(void) {
	try {

		utils::LibManager lib_manager;

		// App app{};
		// app.run();

		Test t1{ 3 };

		Test t2{ 5 };

		std::swap(t1, t2);
	}
	catch (const std::exception& e) {
		fprintf(stderr, "EXCEPTION: %s\n", e.what());
		return -1;
	}
	return 0;
}