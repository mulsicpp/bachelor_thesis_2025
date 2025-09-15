#include "App.h"

#include "utils/defines.h"

#include <algorithm>

#include "scene/Scene.h"

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
        .swapchain_builder(swapchain_builder)
        .build();

    rasterizer = RasterizerBuilder()
        .color_attachment(frame_manager.get_swapchain_attachment())
        .depth_attachment(frame_manager.get_depth_attachment())
        .build()
        .to_shared();

    skinner = SkinnerBuilder().build();

    frame_manager.bind_rasterizer(rasterizer);

    camera = ptr::make_shared<AppCamera>();

    // scene = ptr::make_shared<Scene>(Scene::load("assets/scenes/BrainStem/glTF/BrainStem.gltf"));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));
    scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/space_station_3/scene.gltf"));

    time = std::chrono::high_resolution_clock::now();

    auto& animation = scene->get_animation(0);
    animation.apply_for(0.0f);
    scene->update_transforms();
    scene->skin_cpu();

    rasterizer->bind_scene(scene);

    skinner.bind_scene(scene);
}

App::~App() {
    glfwDestroyWindow(window);
}

void App::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera->aspect = ((float)width) / ((float)height);

        rasterizer->bind_camera(camera);
        auto new_time = std::chrono::high_resolution_clock::now();

        float elapsed = std::chrono::duration<float, std::chrono::seconds::period>(new_time - time).count();

        auto& animation = scene->get_animation(0);
        animation.apply_for(elapsed * 1.0f);
        scene->update_transforms();
        skinner.skin_scene();

        frame_manager.draw();
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
    app->camera->distance = start_distance * glm::pow(base, input_data.zoom_exp);
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
        transform = glm::rotate(transform, app->camera->phi, glm::vec3{ 1.0f, 0.0f, 0.0f });
        transform = glm::rotate(transform, app->camera->theta, glm::vec3{ 0.0f, 1.0f, 0.0f });

        glm::vec3 right = glm::inverse(transform) * glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 up = glm::inverse(transform) * glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f };

        auto delta = (float)dx * right + (float)dy * up;

        app->camera->center -= delta * (float)(app->camera->distance / height);
    }
    else {
        app->camera->theta += dx * move_factor / height;
        app->camera->phi -= dy * move_factor / height;
        app->camera->phi = std::clamp(app->camera->phi, -glm::pi<float>() / 2, glm::pi<float>() / 2);
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
        *app->camera = AppCamera{};

        input_data.zoom_exp = 0.0f;
    }
}