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

    vk::SwapchainBuilder swapchain_builder = vk::SwapchainBuilder().image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    frame_manager = FrameManagerBuilder()
        .max_frames_in_flight(2)
        .swapchain_builder(swapchain_builder)
        .build();

    rasterizer = RasterizerBuilder()
        .color_attachment(frame_manager.get_swapchain_attachment())
        .build();

    frame_manager.bind_rasterizer(&rasterizer);
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        frame_manager.draw_next(&rasterizer, &Rasterizer::draw_triangle_recorder);
    }

    vk::Context::get()->wait_device_idle();
}