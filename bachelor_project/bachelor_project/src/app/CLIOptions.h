#pragma once

#include "external/CLI11.hpp"

#include <string>
#include <tuple>

class CLIOptions {
public:
    std::string scene_path{};
    std::string target_dir{};
    uint32_t frame_count{};
    std::tuple<uint32_t, uint32_t> resolution{};

private:
    CLI::App app{};

public:
    CLIOptions();
    void parse(int argc, char* argv[]);
};