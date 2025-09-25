#include "RtxApp.h"

#include "utils/defines.h"

#include <algorithm>
#include <filesystem>

#include "vk_core/format.h"

#include "scene/Scene.h"
#include "scene/SceneUpdateStrat.h"

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

    raytracer = RaytracerBuilder().build();
    skinner = SkinnerBuilder().build();

    scene = ptr::make_shared<Scene>(Scene::load(get_scene_path(opts.scene)));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));
    scene->update_transforms();
    scene->build_acceleration_structures(opts.fast_build);

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

    RtxPushConstant rtx_push{};

    rtx_push.light_direction = get_scene_light_dir(opts.scene);
    rtx_push.sample_factor = opts.sample_factor;

    auto draw_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        image->cmd_transition(cmd_buf, vk::ImageState::Undefined, vk::ImageState::RtxOutput);
        raytracer.cmd_trace(cmd_buf, opts.pipeline, rtx_push);
        image->cmd_transition(cmd_buf, vk::ImageState::RtxOutput, vk::ImageState::TransferSrc);
        };

    auto skin_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        skinner.cmd_skin_scene(cmd_buf);
        };

    auto update_strats = SceneUpdateStrat::strats();

    std::filesystem::create_directories(std::filesystem::path(opts.output_file).parent_path());
    if (!opts.store_images.empty()) {
        std::filesystem::create_directories(std::filesystem::path(opts.store_images));
    }

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

        if (opts.rebuild_frequency != 0 && (i % opts.rebuild_frequency) == 0) {
            scene->rebuild_acceleration_structures();
        }
        else {
            scene->refit_acceleration_structures();
        }
        frame_benchmark.rebuilt_acc = FrameBenchmark::now();

        cmd_buffer_raytracing.submit().wait();
        frame_benchmark.traced_rays = FrameBenchmark::now();

        printf("rebuild time: %lf ms\n", frame_benchmark.rebuild_acc_time());
        printf("trace time: %lf ms\n", frame_benchmark.trace_rays_time());
        printf("total time: %lf ms\n\n", frame_benchmark.total_time());

        frame_benchmarks.push_back(frame_benchmark);

        if (!opts.store_images.empty()) {
            image->store_in_file((std::filesystem::path(opts.store_images) / ("raytrace_result_" + std::to_string(i) + ".png")).string());
        }
    }


    FILE* output_file = fopen(opts.output_file.c_str(), "w");

    auto scene_name = get_scene_name(opts.scene);
    fprintf(output_file, "scene;%s\n", scene_name.c_str());

    switch (opts.rebuild_frequency) {
    case 0:
        fprintf(output_file, "rebuild;never\n");
        break;
    case 1:
        fprintf(output_file, "rebuild;always\n");
        break;
    default:
        fprintf(output_file, "rebuild;every %u frames\n", opts.rebuild_frequency);
        break;
    }

    fprintf(output_file, "delta time;%f\n", opts.delta_time);

    fprintf(output_file, "pipeline;%s\n", opts.pipeline == RtxPipelineType::Normal ? "normal" : opts.pipeline == RtxPipelineType::Basic ? "basic" : "shadow");

    const auto& [image_width, image_height] = opts.resolution;
    fprintf(output_file, "resolution;%u * %u\n", image_width, image_height);
    fprintf(output_file, "spp;%u\n\n", opts.sample_factor * opts.sample_factor);

    fprintf(output_file, "update;");
    for (const auto& benchmark : frame_benchmarks) {
        fprintf(output_file, "%lf;", benchmark.rebuild_acc_time());
    }
    fprintf(output_file, "\n");

    fprintf(output_file, "trace;");
    for (const auto& benchmark : frame_benchmarks) {
        fprintf(output_file, "%lf;", benchmark.trace_rays_time());
    }
    fprintf(output_file, "\n");

    fclose(output_file);

    vk::Context::get()->wait_device_idle();
}