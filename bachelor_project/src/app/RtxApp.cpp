#include "RtxApp.h"

#include "utils/defines.h"

#include <algorithm>
#include <filesystem>

#include "vk_core/format.h"

#include "scene/Scene.h"
#include "scene/SceneUpdateStrat.h"

#include "stb_image_write.h"

#include "utils/FrameBenchmark.h"

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

    raytracer = RaytracerBuilder().ray_depth(opts.ray_depth + 1).build();
    skinner = SkinnerBuilder().build();

    printf("created raytracer\n");

    scene = ptr::make_shared<Scene>(Scene::load(get_scene_path(opts.scene)));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));
    scene->update_transforms();
    scene->build_acceleration_structures(opts.fast_build);

    printf("loaded scene\n");

    camera = ptr::make_shared<AppCamera>(get_scene_camera(opts.scene));

    camera->aspect = ((float)image->extent().width) / ((float)image->extent().height);
    camera->relative_lens_radius = opts.relative_lens_radius;

    raytracer.bind_camera(camera);
    raytracer.bind_scene(scene);
    raytracer.bind_image(image_view);

    printf("bound\n");

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
    bool animated = scene->animation_count() > 0;
    Animation* animation = nullptr;
    if (animated) {
        animation = &scene->get_animation(0);
    }

    RtxPushConstant rtx_push{};

    rtx_push.light_direction = get_scene_light_dir(opts.scene);
    rtx_push.light_radius = opts.light_radius * glm::pi<float>();

    // rtx_push.ambient_color = glm::vec3(0.0f);
    rtx_push.sample_factor = opts.sample_factor;
    rtx_push.ray_depth = opts.ray_depth;

    rtx_push.flags = 0;
    rtx_push.flags |= opts.greedy ? RTX_PUSH_GREEDY : 0;

    auto draw_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        image->cmd_transition(cmd_buf, vk::ImageState::Undefined, vk::ImageState::RtxOutput);
        raytracer.cmd_trace(cmd_buf, opts.pipeline, rtx_push);
        image->cmd_transition(cmd_buf, vk::ImageState::RtxOutput, vk::ImageState::TransferSrc);
        };

    auto skin_recorder = [&](vk::ReadyCommandBuffer cmd_buf) {
        skinner.cmd_skin_scene(cmd_buf);
        };

    auto update_strats = SceneUpdateStrat::strats();

    if (std::filesystem::path(opts.output_file).has_parent_path()) {
        std::filesystem::create_directories(std::filesystem::path(opts.output_file).parent_path());
    }

    if (!opts.store_images.empty()) {
        std::filesystem::create_directories(std::filesystem::path(opts.store_images));
    }

    std::vector<utils::FrameBenchmark> frame_benchmarks{};

    for (uint32_t i = 0; i < opts.frame_count; i++)
    {
        utils::FrameBenchmark frame_benchmark{};
        printf("drawing frame: %u\n", i);

        if (animated) {
            animation->apply_for(i * opts.delta_time);
            scene->update_transforms();
            if (opts.cpu_skinning) {
                scene->skin_cpu();
            }
            else {
                cmd_buffer_graphics.record(skin_recorder).submit().wait();
            }

            
            if (opts.rebuild_frequency != 0 && (i % opts.rebuild_frequency) == 0) {
                scene->rebuild_acceleration_structures(&frame_benchmark.update);
            }
            else {
                scene->refit_acceleration_structures(&frame_benchmark.update);
            }
        }
        cmd_buffer_raytracing.record(draw_recorder);

        auto start_time = utils::FrameBenchmark::now();
        cmd_buffer_raytracing.submit().wait();
        frame_benchmark.trace = utils::FrameBenchmark::duration(start_time, utils::FrameBenchmark::now());

        printf("rebuild time: %lf ms\n", frame_benchmark.update);
        printf("trace time: %lf ms\n", frame_benchmark.trace);
        printf("total time: %lf ms\n\n", frame_benchmark.total());

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
    fprintf(output_file, "fast %s\n", opts.fast_build ? "build" : "trace");

    fprintf(output_file, "delta time;%f\n", opts.delta_time);

    fprintf(output_file, "pipeline;%s\n", opts.pipeline == RtxPipelineType::Normal ? "normal" : opts.pipeline == RtxPipelineType::Basic ? "basic" : "shadow");

    const auto& [image_width, image_height] = opts.resolution;
    fprintf(output_file, "resolution;%ux%u\n", image_width, image_height);
    fprintf(output_file, "spp;%u\n\n", opts.sample_factor * opts.sample_factor);

    double avg_update_time = 0.0;
    double avg_trace_time = 0.0;
    double avg_total_time = 0.0;

    fprintf(output_file, "update\n");
    for (const auto& benchmark : frame_benchmarks) {
        fprintf(output_file, "%lf;", benchmark.update);
        avg_update_time += benchmark.update;
    }
    fprintf(output_file, "\n");

    fprintf(output_file, "trace\n");
    for (const auto& benchmark : frame_benchmarks) {
        fprintf(output_file, "%lf;", benchmark.trace);
        avg_trace_time += benchmark.trace;
    }
    fprintf(output_file, "\n");

    fprintf(output_file, "total\n");
    for (const auto& benchmark : frame_benchmarks) {
        fprintf(output_file, "%lf;", benchmark.total());
        avg_total_time += benchmark.total();
    }
    fprintf(output_file, "\n\n");

    avg_update_time /= frame_benchmarks.size();
    avg_trace_time /= frame_benchmarks.size();
    avg_total_time /= frame_benchmarks.size();

    fprintf(output_file, "avg. update;%lf\n", avg_update_time);
    fprintf(output_file, "avg. trace;%lf\n", avg_trace_time);
    fprintf(output_file, "avg. total;%lf\n", avg_total_time);

    fclose(output_file);

    vk::Context::get()->wait_device_idle();
}