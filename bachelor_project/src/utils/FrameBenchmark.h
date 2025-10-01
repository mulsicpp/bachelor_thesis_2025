#pragma once

#include <chrono>

namespace utils {
    struct FrameBenchmark {
        using TimePoint = std::chrono::high_resolution_clock::time_point;

        double update{ 0 };

        double trace{ 0 };

        static inline TimePoint now() { return std::chrono::high_resolution_clock::now(); }
        static inline double duration(TimePoint start, TimePoint end) {
            return std::chrono::duration<double, std::chrono::milliseconds::period>(end - start).count();
        }

        inline double total() const { return update + trace; }
    };
}