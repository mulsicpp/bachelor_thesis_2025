#pragma once
#include <memory>

namespace ptr {
	template<class T>
	using Owned = std::unique_ptr<T>;

	template<class T>
	using Shared = std::shared_ptr<T>;

	template<class T>
	using Weak = std::weak_ptr<T>;

	using std::make_shared;

	template <typename T>
	class ToShared {
	public:
		Shared<T> to_shared()&& {
			return make_shared<T>(std::move(static_cast<T&>(*this)));
		}
	};
}