#pragma once

namespace vk {
	template<class T>
	class Handle {
	protected:
		T m_handle = VK_NULL_HANDLE;
	public:
		inline T handle() const {
			return m_handle;
		}
	};
}