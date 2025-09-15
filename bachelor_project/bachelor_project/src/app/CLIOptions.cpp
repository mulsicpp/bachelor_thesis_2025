#include "CLIOptions.h"

#include "utils/defines.h"

#include <cstdio>
#include <cstdlib>

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

    auto validate_dir = CLI::Validator(
        [](const std::string& s) {
            return std::string{};
        },
        "DIR"
    );

    const std::map<std::string, TestScene> scene_map{
        {get_scene_name(TestScene::Brainstem), TestScene::Brainstem},
        {get_scene_name(TestScene::Whirlwind), TestScene::Whirlwind},
        {get_scene_name(TestScene::SpaceStation), TestScene::SpaceStation},
        {get_scene_name(TestScene::Monsters), TestScene::Monsters}
    };


    app.add_option("-s, --scene", scene, "The test scene to be loaded")
        ->transform(CLI::CheckedTransformer(scene_map))
        ->default_str(get_scene_name(scene));

    app.add_option("-t,--target-dir", target_dir, "The path to the directory, where the data is stored")
        ->default_str(target_dir)
        ->check(validate_dir);

    app.add_option("-f,--frames", frame_count, "The number of frames to be generated")
        ->default_str(std::to_string(frame_count))
        ->check(validate_positive_int);

    app.add_option("-r,--resolution", resolution, "The resolution of the frames")
        ->default_str(std::to_string(std::get<0>(resolution)) + " " + std::to_string(std::get<1>(resolution)))
        ->check(validate_positive_int);

    app.add_option("-d,--delta-time", delta_time, "The time that passes between two frames")
        ->default_str(std::to_string(delta_time))
        ->check(validate_positive_float);

    const std::map<std::string, bool> skinning_map{
        {"cpu", true},
        {"gpu", false}
    };

    app.add_option("--skin-mode", cpu_skinning, "The mode in which models are skinned when animated")
        ->transform(CLI::CheckedTransformer(skinning_map))
        ->default_str(cpu_skinning ? "cpu" : "gpu");

    app.add_flag("--store-images", store_images, "Store the rendered frames as PNG images");

    app.set_version_flag("-v,-V,--version", APP_NAME " 1.0.0");
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