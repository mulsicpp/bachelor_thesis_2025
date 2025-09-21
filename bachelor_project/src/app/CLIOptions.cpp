#include "CLIOptions.h"

#include "utils/defines.h"
#include "utils/AppPath.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

CLIOptions::CLIOptions() : app{ APP_DESCRIPTION, APP_NAME } {
    auto validate_positive_int = CLI::Validator(
        [](const std::string& s) {
            int64_t val = std::stoll(s);

            if (val <= 0) return s + " is not a positive value";
            return std::string{};
        },
        "POSITIVE"
    );

    auto validate_positive_float = CLI::Validator(
        [](const std::string& s) {
            double val = std::stod(s);

            if (val <= 0.0) return s + " is not a positive value";
            return std::string{};
        },
        "POSITIVE"
    );

    auto validate_file = CLI::Validator(
        [](const std::string& s) {
            return std::string{};
        },
        "FILE"
    );

    auto validate_square_number = CLI::Validator(
        [](const std::string& s) {
            int64_t val = std::stoll(s);

            if (val <= 0) return s + " is not a square number";

            auto root = std::llround(std::sqrt(val));

            if (root * root != val) return s + " is not a square number";
            return std::string{};
        },
        "SQUARE NUMBER"
    );

    const std::vector<TestScene> scene_values = {
        TestScene::Brainstem,
        TestScene::Whirlwind,
        TestScene::SpaceStation,
        TestScene::Monsters,
    };

    std::vector<std::string> scene_str_values{};

    for (const auto val : scene_values) {
        scene_str_values.push_back(get_scene_name(val));
    }


    auto scene_callback = [this, scene_str_values, scene_values](const std::string& scene_str) {
        for (uint32_t i = 0; i < scene_values.size(); i++) {
            if (scene_str_values[i] == scene_str) {
                scene = scene_values[i];
                return;
            }
        }
        };

    auto spp_callback = [this](const uint32_t& spp) {
        auto root = std::llround(std::sqrt(spp));
        sample_factor = root;
        };


    app.add_option_function<std::string>("-s, --scene", scene_callback, "The test scene to be loaded")
        ->check(CLI::IsMember(scene_str_values))
        ->default_str(get_scene_name(scene));

    app.add_option("-o,--output-file", output_file, "The path to the output file, where the data is stored")
        ->default_str(output_file)
        ->check(validate_file);

    app.add_option("-f,--frames", frame_count, "The number of frames to be generated")
        ->default_str(std::to_string(frame_count))
        ->check(validate_positive_int);

    app.add_option("-r,--resolution", resolution, "The resolution of the frames")
        ->default_str(std::to_string(std::get<0>(resolution)) + " " + std::to_string(std::get<1>(resolution)))
        ->check(validate_positive_int);

    app.add_option("-d,--delta-time", delta_time, "The time that passes between two frames")
        ->default_str(std::to_string(delta_time))
        ->check(validate_positive_float);

    app.add_option_function<uint32_t>("--spp", spp_callback, "Samples per pixel aka. the number of primary rays cast per pixel")
        ->default_str(std::to_string(sample_factor * sample_factor))
        ->check(validate_square_number);

    app.add_flag("--cpu-skin,!--gpu-skin", cpu_skinning, "The mode in which models are skinned when animated");

    app.add_flag("--store-images", store_images, "Store the rendered frames as PNG images");

    app.add_flag("--shadows,!--no-shadows", shadows, "Enable shadow testing");

    app.set_version_flag("-v,-V,--version", utils::AppPath::instance().app_name + " 1.0.0");
}

void CLIOptions::parse(int argc, char* argv[]) {
    try {
        app.parse(argc, argv);
    }
    catch (const CLI::CallForHelp& e) {
        exit(app.exit(e));
    }
    catch (const CLI::ParseError& e) {
        exit(app.exit(e));
    }
}