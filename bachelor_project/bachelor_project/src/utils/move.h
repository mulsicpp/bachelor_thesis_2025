#pragma once

#include "ptr.h"

#define MOVE_SEMANTICS_DEBUG
#ifdef MOVE_SEMANTICS_DEBUG
#include "dbg_log.h"
#define MOVE_LOG(...) dbg_log(__VA_ARGS__)
#else
#define MOVE_LOG(...)
#endif

#define MOVE_SEMANTICS(class_name)                     					\
public:                                                					\
	inline class_name(class_name&& other) noexcept {   					\
		MOVE_LOG(#class_name " move");                                  \
		move_members(other);	                                 		\
		other.mark_moved();                            					\
	}                                                 					\
																		\
	inline class_name& operator=(class_name&& other) noexcept {			\
		if (this == &other) return *this;								\
																		\
		if (!was_moved()) {												\
			destroy();													\
		}																\
		MOVE_LOG(#class_name " move");                                  \
		move_members(other);											\
		other.mark_moved();												\
																		\
		return *this;													\
	}																	\
																		\
	inline ~class_name() {										        \
		if (was_moved()) return;										\
		MOVE_LOG(#class_name " destroy");                               \
		destroy();														\
	}																	\
																		\
	inline ptr::Shared<class_name> to_shared() {						\
		return ptr::make_shared<class_name>(std::move(*this));          \
	}																	\
																		\
private:																\
	class_name(const class_name& other) = default;						\
	class_name& operator=(const class_name& other) = default;			


#define MOVE_MARKER(handle, value)										\
private:																\
	inline void mark_moved() { handle = value; }						\
	inline bool was_moved() { return handle == value; }

#define MOVE_MARKER_VK_HANDLE(handle) MOVE_MARKER(handle, VK_NULL_HANDLE)
#define DEFAULT_MOVE(class_name) private: void move_members(const class_name& other) { *this = other; }

#define MOVE_SEMANTICS_VK_DEFAULT(class_name, handle) MOVE_MARKER_VK_HANDLE(handle) DEFAULT_MOVE(class_name) MOVE_SEMANTICS(class_name)

namespace utils {

	class Move {
	public:
		Move() = default;
		~Move() = default;

	protected:
		Move(const Move& other) = delete;
		Move(Move&& other) = default;

		Move& operator=(const Move&) = delete;
		Move& operator=(Move&&) = default;
	};
}
