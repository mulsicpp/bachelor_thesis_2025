#include "RtxApp.h"

#include "utils/defines.h"

#include <algorithm>

#include "vk_core/format.h"

#include "scene/Scene.h"

#include "stb_image_write.h"

#include "app/FrameBenchmark.h"

RtxApp::RtxApp(int argc, char* argv[])
{
    opts.parse(argc, argv);

    auto context_info = vk::ContextInfo()
        .app_name(APP_NAME)
        .use_raytracing(true);

    vk::Context::create(context_info);

    const auto& [image_width, image_height] = opts.resolution;

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

    raytracer = RaytracerBuilder().shadows(true).build();
    skinner = SkinnerBuilder().build();

    scene = ptr::make_shared<Scene>(Scene::load(get_scene_path(opts.scene)));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));
    scene->update_transforms();
    scene->build_acceleration_structures();

    camera = ptr::make_shared<AppCamera>(get_scene_camera(opts.scene));

    camera->aspect = ((float)IMAGE_WIDTH) / ((float)IMAGE_HEIGHT);

    raytracer.bind_camera(camera);
    raytracer.bind_scene(scene);
    raytracer.bind_image(image_view);

    skinner.bind_scene(scene);

    // vk::Buffer buffer = vk::BufferBuilder()
    //     .usage(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
    //     .add_queue_type(vk::QueueType::Transfer)
    //     .memory_usage(VMA_MEMORY_USAGE_CPU_ONLY)
    //     .size(image->extent().width * image->extent().height * 4)
    //     .build();

    cmd_buffer_raytracing = vk::CommandBufferBuilder().queue_type(vk::QueueType::Compute).build();
    cmd_buffer_graphics = vk::CommandBufferBuilder().queue_type(vk::QueueType::Graphics).build();

    // auto* data = buffer.mapped_data<uint8_t>();
    // stbi_write_png("raytrace_result.png", image->extent().width, image->extent().height, 4, buffer.mapped_data<void>(), 0);
}

void RtxApp::run()
{
    auto& animation = scene->get_animation(0);

    auto draw_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        image->cmd_transition(cmd_buf, vk::ImageState::Undefined, vk::ImageState::RtxOutput);
        raytracer.cmd_draw(cmd_buf);
        image->cmd_transition(cmd_buf, vk::ImageState::RtxOutput, vk::ImageState::TransferSrc);
        };

    auto skin_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        skinner.cmd_skin_scene(cmd_buf);
        };

    if (opts.store_images) {
        for (uint32_t i = 0; i < opts.frame_count; i++)
        {
            printf("drawing frame: %u\n", i);
            animation.apply_for(i * opts.delta_time);
            scene->update_transforms();
            if (opts.cpu_skinning) {
                scene->skin_cpu();
            }
            else {
                cmd_buffer_graphics.record(skin_recorder).submit().wait();
            }
            scene->rebuild_acceleration_structures();

            cmd_buffer_raytracing.record(draw_recorder).submit().wait();
            image->store_in_file("raytrace_result_" + std::to_string(i) + ".png");
        }
    } else {

        std::vector<FrameBenchmark> frame_benchmarks{}; 
        for (uint32_t i = 0; i < opts.frame_count; i++)
        {
            FrameBenchmark frame_benchmark{};
            printf("drawing frame: %u\n", i);
            animation.apply_for(i * opts.delta_time);
            scene->update_transforms();
            if (opts.cpu_skinning) {
                scene->skin_cpu();
            }
            else {
                cmd_buffer_graphics.record(skin_recorder).submit().wait();
            }

            cmd_buffer_raytracing.record(draw_recorder);
            frame_benchmark.start = FrameBenchmark::now();
            scene->refit_acceleration_structures();
            frame_benchmark.rebuilt_acc = FrameBenchmark::now();

            cmd_buffer_raytracing.submit().wait();
            frame_benchmark.traced_rays = FrameBenchmark::now();

            printf("rebuild time: %lf ms\n", frame_benchmark.rebuild_acc_time());
            printf("trace time: %lf ms\n", frame_benchmark.trace_rays_time());
            printf("total time: %lf ms\n\n", frame_benchmark.total_time());
        }
    }

    vk::Context::get()->wait_device_idle();
}