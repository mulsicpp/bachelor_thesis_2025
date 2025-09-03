#include "CLIOptions.h"

#include "utils/defines.h"

#include <cstdio>
#include <cstdlib>

CLIOptions::CLIOptions() : app{ APP_DESCRIPTION, APP_NAME } {
    auto positive_int = CLI::Validator(
    [](const std::string &s) {
        int64_t val = std::stoll(s);

        if (val <= 0) return s + " is not a positive value";
        return std::string{};
    },
    "PositiveInt"
);


    app.add_option("scene", scene_path, "The path to the scene to be loaded")
        ->check(CLI::ExistingPath)
        ->required(true);
    
    app.add_option("-t,--target-dir", target_dir, "The path to the directory, where the data is stored")
        ->default_val("raytracing_results");

    app.add_option("-f,--frames", frame_count, "The number of frames to be generated")
        ->check(positive_int)
        ->default_val(100);

    app.add_option("-r,--resolution", resolution, "The resolution of the frames")
        ->check(positive_int)
        ->default_str(DEFAULT_RESOLUTION_STR);

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