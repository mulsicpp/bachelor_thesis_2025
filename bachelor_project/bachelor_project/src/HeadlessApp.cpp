#include "HeadlessApp.h"

#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT  720

#define APP_NAME "Raytracing App"

#include <algorithm>

#include "scene/Scene.h"

HeadlessApp::HeadlessApp() {
    auto context_info = vk::ContextInfo()
        .app_name(APP_NAME)
        .use_raytracing(false);

    vk::Context::create(context_info);

    // TODO
    /*
    rasterizer = RasterizerBuilder()
        .color_attachment(frame_manager.get_swapchain_attachment())
        .depth_attachment(frame_manager.get_depth_attachment())
        .build()
        .to_shared();*/

    scene = ptr::make_shared<Scene>(Scene::load("assets/scenes/BrainStem/glTF/BrainStem.gltf"));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));

    dynamic_buffer = vk::BufferBuilder()
        .usage(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        .memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
        .queue_types({ vk::QueueType::Graphics, vk::QueueType::Transfer })
        .size(scene->get_dynamic_buffer_size())
        .build().to_shared();

    scene->select_dynamic_buffer(dynamic_buffer);

    auto& animation = scene->get_animation(0);
    animation.apply_for(0.0f);
    scene->update();
}

void HeadlessApp::run() {
    dbg_log("run");
    for (uint32_t i = 0; i < 20; i++) {
        dbg_log("run iteration %u", i);
    }

    vk::Context::get()->wait_device_idle();
}