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


    app.add_option("scene", scene_path, "The path to the scene to be loaded")
        ->check(CLI::ExistingFile)
        ->required(true);

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