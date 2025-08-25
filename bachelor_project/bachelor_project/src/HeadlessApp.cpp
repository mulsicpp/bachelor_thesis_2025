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