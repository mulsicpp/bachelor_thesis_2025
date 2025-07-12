#pragma once

namespace utils {

	class NoCopy {
	public:
		NoCopy() = default;

		NoCopy(const NoCopy& other) = delete;
		NoCopy(NoCopy&& other) = delete;

		NoCopy& operator=(const NoCopy&) = delete;
		NoCopy& operator=(NoCopy&&) = delete;
	};
}