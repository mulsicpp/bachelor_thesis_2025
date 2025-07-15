#pragma once

#include "dbg_log.h"

#define MOVE_SEMANTICS(class_name)                     					\
public:                                                					\
	inline class_name(class_name&& other) noexcept {   					\
		*this = other;                                 					\
		other.mark_moved();                            					\
	}                                                 					\
																		\
	inline class_name& operator=(class_name&& other) noexcept {			\
		if (this == &other) return *this;								\
																		\
		if (!was_moved()) {												\
			destroy();													\
		}																\
		*this = other;													\
		other.mark_moved();												\
																		\
		return *this;													\
	}																	\
																		\
	inline ~class_name() {										        \
		if (was_moved()) return;										\
		destroy();														\
		mark_moved();													\
	}																	\
																		\
private:																\
	class_name(const class_name& other) = default;						\
	class_name& operator=(const class_name& other) = default;			\
