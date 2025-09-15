#pragma once

#include <filesystem>

namespace utils {

    namespace fs = std::filesystem;

    struct AppPath {
    public:
        fs::path app_path{};
        fs::path app_dir{};
        std::string app_name{};

        fs::path get_path(const fs::path& path) const;

        static const AppPath& instance();
    private:
        static AppPath* _instance;

        AppPath();

        AppPath& operator=(const AppPath&) = delete;
        AppPath& operator=(AppPath&&) = delete;
    };
}