#include "App.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

App::App() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);
    m_context = vk::Context::create(m_window, APP_NAME);
}

App::~App() {
    glfwDestroyWindow(m_window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}