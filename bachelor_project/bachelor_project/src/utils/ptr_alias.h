#pragma once
#include <memory>

namespace ptr {
	template<class T>
	using Owned = std::unique_ptr<T>;

	template<class T>
	using Shared = std::shared_ptr<T>;

	template<class T>
	using Weak = std::weak_ptr<T>;
}