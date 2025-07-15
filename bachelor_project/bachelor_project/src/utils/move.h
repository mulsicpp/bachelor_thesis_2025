#pragma once

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
	}																	\
																		\
private:																\
	class_name(const class_name& other) = default;						\
	class_name& operator=(const class_name& other) = default;			


#define MOVE_MARKER(handle, value)										\
inline void mark_moved() { handle = value; }							\
inline bool was_moved() { return handle == value; }

#define MOVE_MARKER_VK_HANDLE(handle) MOVE_MARKER(handle, VK_NULL_HANDLE)

#define MOVE_SEMANTICS_VK_HANDLE(class_name, handle) MOVE_MARKER_VK_HANDLE(handle) MOVE_SEMANTICS(class_name)