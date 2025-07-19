#include "App.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

App::App() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);
    
    auto context_info = vk::ContextInfo()
        .window(window)
        .app_name(APP_NAME)
        .use_raytracing(false);

    vk::Context::create(context_info);

    rasterizer = RasterizerBuilder().build();
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        rasterizer.draw_triangle();
    }

    vk::Context::get()->wait_device_idle();
}