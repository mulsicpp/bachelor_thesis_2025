#include "LibManager.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

#include "vk_core/Context.h"


namespace utils {
    LibManager::LibManager() {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }
    }

    LibManager::~LibManager() {
        vk::Context::destroy();
        glfwTerminate();
    }
}