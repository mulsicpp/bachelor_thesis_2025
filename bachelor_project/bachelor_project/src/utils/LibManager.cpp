#include "LibManager.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace utils {
    LibManager::LibManager() {
        if (!glfwInit()) {
            throw std::runtime_error("GLFW init failed");
        }
    }

    LibManager::~LibManager() {
        glfwTerminate();
    }
}