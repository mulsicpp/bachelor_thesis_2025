#include "App.h"

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT  720

#define APP_NAME "Raytracing App"

#include <algorithm>

#include "scene/Scene.h"

CameraUBO AppCamera::as_camera_ubo() const {
    glm::mat4 view = glm::mat4{ 1.0f };
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -distance));
    view = glm::rotate(view, phi, glm::vec3{ 1.0f, 0.0f, 0.0f });
    view = glm::rotate(view, theta, glm::vec3{ 0.0f, 1.0f, 0.0f });
    view = glm::translate(view, -center);
    view = glm::scale(view, glm::vec3{ 1.0f, -1.0f, -1.0f });

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, near * distance * 0.2f, far);

    return CameraUBO{ view, proj };
}

App::App() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, APP_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    
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
        .depth_attachment(frame_manager.get_depth_attachment())
        .build()
        .to_shared();

    frame_manager.bind_rasterizer(rasterizer);

    scene = ptr::make_shared<Scene>(Scene::load("assets/scenes/Avocado/glTF/Avocado.gltf"));
    scene->update();
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    dbg_log("run");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Frame& frame = *frame_manager.get_current_frame();

        //camera.theta = 3.0f * glm::pi<float>() / 4.0f;
        //camera.phi = -glm::pi<float>() / 4.0f;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera.aspect = ((float)width) / ((float)height);

        *frame.p_camera_ubo = camera.as_camera_ubo();
        frame.scene = scene;
        frame_manager.draw_frame();
    }

    vk::Context::get()->wait_device_idle();
}


static struct {
    double last_x, last_y;
    float zoom_exp = 0.0f;
} input_data;



void App::scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
    static float start_distance = AppCamera{}.distance;
    static float base = 1.2f;

    input_data.zoom_exp -= y_offset;

    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    app->camera.distance = start_distance * glm::pow(base, input_data.zoom_exp);
}

void App::cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    static double move_factor = 5.0;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) return;

    double dx = x_pos - input_data.last_x;
    double dy = y_pos - input_data.last_y;

    input_data.last_x = x_pos;
    input_data.last_y = y_pos;

    auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    double height = (double)app->frame_manager.get_framebuffer_extent().height;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        glm::mat4 transform = glm::mat4{ 1.0f };
        transform = glm::rotate(transform, app->camera.phi, glm::vec3{ 1.0f, 0.0f, 0.0f });
        transform = glm::rotate(transform, app->camera.theta, glm::vec3{ 0.0f, 1.0f, 0.0f });

        glm::vec3 right = glm::inverse(transform) * glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 up = glm::inverse(transform) * glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f };

        auto delta = (float)dx * right + (float)dy * up;

        app->camera.center -= delta * (float)(app->camera.distance / height);
    }
    else {
        app->camera.theta += dx * move_factor / height;
        app->camera.phi -= dy * move_factor / height;
        app->camera.phi = std::clamp(app->camera.phi, -glm::pi<float>() / 2, glm::pi<float>() / 2);
    }

}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwGetCursorPos(window, &input_data.last_x, &input_data.last_y);
    }
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->camera = AppCamera{};

        input_data.zoom_exp = 0.0f;
    }
}