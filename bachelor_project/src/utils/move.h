#pragma once

#include "ptr.h"

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
