#pragma once

#include "external/CLI11.hpp"

#include <string>
#include <tuple>

#include "utils/defines.h"

class CLIOptions {
public:
    std::string scene_path{};
    std::string target_dir{ "raytracing_results" };
    uint32_t frame_count{ 200 };
    std::tuple<uint32_t, uint32_t> resolution{ std::tuple<uint32_t, uint32_t>(IMAGE_WIDTH, IMAGE_HEIGHT) };
    float delta_time{ 0.1f };

private:
    CLI::App app{};

public:
    CLIOptions();
    void parse(int argc, char* argv[]);
};