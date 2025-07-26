#include "App.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

CameraUBO AppCamera::as_camera_ubo() const {
    glm::mat4 view = glm::mat4{ 1.0f };
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -distance));
    view = glm::rotate(view, phi, glm::vec3{ 1.0f, 0.0f, 0.0f });
    view = glm::rotate(view, theta, glm::vec3{ 0.0f, 1.0f, 0.0f });
    view = glm::translate(view, -center);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 20.0f);

    return CameraUBO{ view, proj };
}

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
        .build()
        .to_shared();

    frame_manager.bind_rasterizer(rasterizer);
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Frame& frame = *frame_manager.get_current_frame();

        camera.theta = glm::pi<float>() / 4.0f;
        camera.phi = glm::pi<float>() / 4.0f;
        *frame.p_camera_ubo = camera.as_camera_ubo();
        frame_manager.draw_frame();
    }

    vk::Context::get()->wait_device_idle();
}