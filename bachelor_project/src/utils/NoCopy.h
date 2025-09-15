#pragma once

namespace utils {

	class NoCopy {
	public:
		NoCopy() = default;

	protected:
		NoCopy(const NoCopy& other) = default;
		NoCopy(NoCopy&& other) = default;

		NoCopy& operator=(const NoCopy&) = default;
		NoCopy& operator=(NoCopy&&) = default;
	};
}