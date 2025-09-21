#include "SceneUpdateStrat.h"

static bool refit_always(uint32_t frame_index) { return false; }
static bool rebuild_always(uint32_t frame_index) { return true; }

template<uint32_t T>
static bool rebuild_every(uint32_t frame_index) { return frame_index % T == 0; }

std::vector<SceneUpdateStrat> SceneUpdateStrat::strats() {
    return {
        {"refit_always", refit_always},
        {"rebuild_always", rebuild_always},
        {"rebuild_every_8_frames", rebuild_every<8>},
        {"rebuild_every_32_frames", rebuild_every<32>}
    };
}