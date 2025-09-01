#include "RtxApp.h"

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480

#define APP_NAME "Raytracing App"

#include <algorithm>

#include "vk_core/format.h"

#include "scene/Scene.h"

RtxApp::RtxApp()
{
    auto context_info = vk::ContextInfo()
                            .app_name(APP_NAME)
                            .use_raytracing(true);

    vk::Context::create(context_info);

    image = vk::ImageBuilder()
                      .extent({IMAGE_WIDTH, IMAGE_HEIGHT})
                      .format(VK_FORMAT_R8G8B8A8_UNORM)
                      .queue_types({vk::QueueType::Transfer, vk::QueueType::Compute})
                      .usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
                      .aspect(VK_IMAGE_ASPECT_COLOR_BIT)
                      .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
                      .tiling(VK_IMAGE_TILING_OPTIMAL)
                      .build()
                      .to_shared();

    image_view = vk::ImageView::create_from(image, image->aspect()).to_shared();

    raytracer = RaytracerBuilder().build();

    scene = ptr::make_shared<Scene>(Scene::load("assets/scenes/BrainStem/glTF/BrainStem.gltf"));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));

    auto &animation = scene->get_animation(0);
    animation.apply_for(0.0f);
    scene->update();
    scene->build_acceleration_structures();

    camera = ptr::make_shared<AppCamera>();

    camera->aspect = ((float)IMAGE_WIDTH) / ((float)IMAGE_HEIGHT);
    camera->theta = glm::pi<float>();
    camera->center = glm::vec3{0.0f, -1.0f, 0.0f};
    camera->distance /= 2;
    
    raytracer.bind_camera(camera);
    dbg_log("bound camera");
    raytracer.bind_scene(scene);
    dbg_log("bound scene");
    raytracer.bind_image(image_view);
    dbg_log("bound image view");

    vk::CommandBuffer::single_time_submit(vk::QueueType::Compute, [&](vk::ReadyCommandBuffer cmd_buf) {
        image->cmd_transition(cmd_buf, vk::ImageState::Undefined, vk::ImageState::TransferSrc);
    });

    // raytracer.draw();
    image->store_in_file("raytrace_result.png");
}

void RtxApp::run()
{
    dbg_log("run");


    for (uint32_t i = 0; i < 20; i++)
    {
        // dbg_log("run iteration %u", i);
    }

    vk::Context::get()->wait_device_idle();
}