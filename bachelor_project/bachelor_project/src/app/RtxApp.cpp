#include "RtxApp.h"

#include "utils/defines.h"

#include <algorithm>

#include "vk_core/format.h"

#include "scene/Scene.h"

#include "stb_image_write.h"

RtxApp::RtxApp(int argc, char* argv[])
{
    opts.parse(argc, argv);

    auto context_info = vk::ContextInfo()
        .app_name(APP_NAME)
        .use_raytracing(true);

    vk::Context::create(context_info);

    const auto&[image_width, image_height] = opts.resolution;

    dbg_log("resolution: %u %u", image_width, image_height);

    image = vk::ImageBuilder()
        .extent({ image_width, image_height })
        .format(VK_FORMAT_R8G8B8A8_UNORM)
        .queue_types({ vk::QueueType::Transfer, vk::QueueType::Compute })
        .usage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT)
        .aspect(VK_IMAGE_ASPECT_COLOR_BIT)
        .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
        .tiling(VK_IMAGE_TILING_OPTIMAL)
        .build()
        .to_shared();

    image_view = vk::ImageView::create_from(image, image->aspect()).to_shared();

    raytracer = RaytracerBuilder().build();

    scene = ptr::make_shared<Scene>(Scene::load(opts.scene_path));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));

    auto& animation = scene->get_animation(0);
    animation.apply_for(0.0f);
    scene->update();
    scene->build_acceleration_structures();

    camera = ptr::make_shared<AppCamera>();

    camera->aspect = ((float)IMAGE_WIDTH) / ((float)IMAGE_HEIGHT);
    camera->theta = glm::pi<float>() * 0.75f;
    camera->phi = glm::pi<float>() * 0.25f;
    camera->center = glm::vec3{ 0.0f, -1.0f, 0.0f };
    camera->distance /= 2;

    raytracer.bind_camera(camera);
    dbg_log("bound camera");
    raytracer.bind_scene(scene);
    dbg_log("bound scene");
    raytracer.bind_image(image_view);
    dbg_log("bound image view");

    // vk::Buffer buffer = vk::BufferBuilder()
    //     .usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    //     .add_queue_type(vk::QueueType::Transfer)
    //     .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
    //     .size(image->extent().width * image->extent().height * 4)
    //     .build();

    vk::CommandBuffer cmd_buf = vk::CommandBufferBuilder(vk::QueueType::Compute).build();

    auto draw_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        image->cmd_transition(cmd_buf, vk::ImageState::Undefined, vk::ImageState::RtxOutput);
        raytracer.cmd_draw(cmd_buf);
        image->cmd_transition(cmd_buf, vk::ImageState::RtxOutput, vk::ImageState::TransferSrc);
        // image->cmd_store(cmd_buf, &buffer);
        };

    cmd_buf.record(draw_recorder).submit().wait();

    image->store_in_file("raytrace_result.png");

    // auto* data = buffer.mapped_data<uint8_t>();
    // stbi_write_png("raytrace_result.png", image->extent().width, image->extent().height, 4, buffer.mapped_data<void>(), 0);
}

void RtxApp::run()
{
    dbg_log("run");


    for (uint32_t i = 0; i < opts.frame_count; i++)
    {
        // dbg_log("run iteration %u", i);
    }

    vk::Context::get()->wait_device_idle();
}