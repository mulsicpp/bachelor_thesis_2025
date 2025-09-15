#pragma once

#include "external/volk.h"
#include "utils/defines.h"


#if DEBUG_MOVE
#include "utils/dbg_log.h"
#include <typeinfo>
#define HANDLE_LOG(msg) dbg_log("%s %p %s", typeid(T).name(), (void*)handle, msg)
#define MOVE_LOG(msg) dbg_log(msg)
#else
#define HANDLE_LOG(msg) {}
#define MOVE_LOG(msg) {}
#endif



namespace vk {

	struct ImageHandle {
		VkImage handle{ VK_NULL_HANDLE };
		bool owns{ true };

		inline ImageHandle(VkImage handle, bool owns = true)
			: handle{handle}
			, owns{owns}
		{}

		inline operator VkImage() const {
			return handle;
		}

		inline operator void*() const {
			return (void*)handle;
		}

		ImageHandle(ImageHandle&&) = default;
		ImageHandle(const ImageHandle&) = default;

		ImageHandle& operator=(ImageHandle&&) = default;
		ImageHandle& operator=(const ImageHandle&) = default;

		inline ImageHandle& operator=(VkImage handle) {
			this->handle = handle;
			return *this;
		}

		inline bool operator!=(VkImage handle) {
			return this->handle != handle;
		}
	};

	template<class T>
	void destroy_handle(T handle);

	template<class T>
	class Handle {
	private:
		T handle;

	public:
		inline Handle() : handle{ VK_NULL_HANDLE } {}
		inline ~Handle() {
			if (handle != VK_NULL_HANDLE) {
				destroy_handle<T>(handle);
				HANDLE_LOG("destroyed");
			}
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
}

#undef HANDLE_LOG