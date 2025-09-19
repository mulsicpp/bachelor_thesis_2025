#pragma once

#include <chrono>

struct FrameBenchmark {
    using TimePoint = std::chrono::high_resolution_clock::time_point;

    TimePoint start{};

    TimePoint rebuilt_acc{};

    TimePoint traced_rays{};

    static inline TimePoint now() { return std::chrono::high_resolution_clock::now(); }
    static inline double duration(TimePoint start, TimePoint end) {
        return std::chrono::duration<double, std::chrono::milliseconds::period>(end - start).count();
    }

    inline double total_time() {
        return duration(start, traced_rays);
    }

    inline double rebuild_acc_time() {
        return duration(start, rebuilt_acc);
    }

    inline double trace_rays_time() {
        return duration(rebuilt_acc, traced_rays);
    }
};