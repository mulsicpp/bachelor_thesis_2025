#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct SceneUpdateStrat {
    std::string name{};
    bool (*rebuild_at)(uint32_t frame_index);

    static std::vector<SceneUpdateStrat> strats();
};