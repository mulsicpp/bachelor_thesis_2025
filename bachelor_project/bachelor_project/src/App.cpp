#include "App.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

App::App() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);
    
    vk::Context::create(window, APP_NAME);

    rasterizer = RasterizerBuilder().build();
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        rasterizer.draw();
    }
}