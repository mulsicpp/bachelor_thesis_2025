#pragma once

#include "external/CLI11.hpp"

#include <string>
#include <tuple>

#include "utils/defines.h"

#include "TestScene.h"
#include "rendering/Raytracer.h"

class CLIOptions {
public:
    TestScene scene{};
    std::string output_file{ "results.csv" };
    uint32_t frame_count{ 200 };
    std::tuple<uint32_t, uint32_t> resolution{ std::tuple<uint32_t, uint32_t>(IMAGE_WIDTH, IMAGE_HEIGHT) };
    float delta_time{ 0.1f };
    uint32_t sample_factor{ 1 };
    RtxPipelineType pipeline{ RtxPipelineType::Normal };
    uint32_t rebuild_frequency { 1 };
    std::string store_images{ "" };

    bool cpu_skinning{ false };
    bool fast_build{ false };

private:
    CLI::App app{};

public:
    CLIOptions();
    void parse(int argc, char* argv[]);
};