#include "AppPath.h"

#if defined(_WIN32)
#include "windows.h"
#elif(__linux__)
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "dbg_log.h"

namespace utils {

    AppPath* AppPath::_instance{ nullptr };

    AppPath::AppPath() {
#if defined(_WIN32)
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
#elif(__linux__)
        char exe_path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", exe_path, PATH_MAX);
        exe_path[count] = 0;
#endif
        dbg_log("path to app: %s", exe_path);
        app_path = fs::canonical(fs::path(std::string(exe_path)));
        app_dir = app_path.parent_path();
        app_name = app_path.filename();
    }

    fs::path AppPath::get_path(const fs::path& path) const {
        const auto cwd = fs::current_path();
        fs::current_path(app_dir);
        const auto path_abs =  fs::absolute(path);
        fs::current_path(cwd);
        return path_abs;
    }

    const AppPath& AppPath::instance() {
        if(_instance == nullptr) {
            _instance = new AppPath{};
        }
        return *_instance;
    }
}