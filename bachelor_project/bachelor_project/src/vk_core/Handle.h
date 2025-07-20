#pragma once

#include <vulkan/vulkan.h>
#include "utils/defines.h"


#if DEBUG_MOVE
#include "utils/dbg_log.h"
#include <typeinfo>
#define HANDLE_LOG(msg) dbg_log("%s %p %s", typeid(T).name(), handle, msg)
#else
#define HANDLE_LOG(msg) {}
#endif



namespace vk {

	struct HImage {
		VkImage handle{ VK_NULL_HANDLE };
		bool owns{ true };

		inline HImage(VkImage handle, bool owns = true) 
			: handle{handle}
			, owns{owns}
		{}

		operator VkImage() const {
			return handle;
		}
	};

	template<class T>
	class Handle {
	private:
		T handle;

	public:
		inline Handle() : handle{ VK_NULL_HANDLE } {}
		inline ~Handle() {
			if(handle != VK_NULL_HANDLE)
				HANDLE_LOG("destroyed");
			destroy_handle<T>(handle);
		}

		inline Handle(Handle<T>&& other) noexcept : handle{ other.handle } {
			other.handle = VK_NULL_HANDLE;
			HANDLE_LOG("moved in constructor");
		}

		inline Handle<T>& operator=(Handle<T>&& other) noexcept {
			if (this == &other) return *this;

			if (handle != VK_NULL_HANDLE) {
				destroy_handle<T>(handle);
				HANDLE_LOG("destroyed");
			}
			handle = other.handle;
			other.handle = VK_NULL_HANDLE;

			HANDLE_LOG("moved in assignment");

			return *this;
		}

		inline Handle(const Handle<T>& other) = delete;
		inline Handle<T>& operator=(const Handle<T>& other) = delete;

		inline T& operator*() { return handle; }
		inline const T& operator*() const { return handle; }

		inline T get() const { return handle; }

		inline void invalidate() { handle = VK_NULL_HANDLE; }
	};

	template<class T>
	void destroy_handle(T handle);
}

#undef HANDLE_LOG