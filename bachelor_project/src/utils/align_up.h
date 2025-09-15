#pragma once

namespace utils {
    template<class T>
    T align_up(T value, T alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }
}