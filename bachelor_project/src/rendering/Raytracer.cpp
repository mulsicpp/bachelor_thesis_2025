#include "Raytracer.h"

#include "utils/dbg_log.h"
#include "utils/AppPath.h"

#include "vk_pipeline/Shader.h"


void Raytracer::bind_camera(const ptr::Shared<Camera>& camera) {
    *camera_uniform_buffer->mapped_data<CameraUBO>() = camera->as_camera_ubo(true);
    camera_uniform_buffer->flush();
}

void Raytracer::bind_scene(const ptr::Shared<Scene>& scene) {
    this->scene = scene;
    descriptor_pool.update_set_binding(0, 0, vk::TlasDescriptorInfo(scene->get_tlas()));

    const auto& buffers = scene->get_buffers();
    descriptor_pool.update_set_binding(0, 3, vk::BufferDescriptorInfo(buffers.indices));
    descriptor_pool.update_set_binding(0, 4, vk::BufferDescriptorInfo(buffers.positions));
    descriptor_pool.update_set_binding(0, 5, vk::BufferDescriptorInfo(buffers.dynamic_positions));
    descriptor_pool.update_set_binding(0, 6, vk::BufferDescriptorInfo(buffers.primitive_offsets));
}

void Raytracer::bind_image(const ptr::Shared<vk::ImageView>& image_view) {
    this->image_view = image_view;
    descriptor_pool.update_set_binding(0, 1, vk::ImageDescriptorInfo(image_view));
}

void Raytracer::cmd_trace(vk::ReadyCommandBuffer cmd_buf, RtxPipelineType type, const RtxPushConstant& rtx_push) {
    vk::RtxPipeline* selected_pipeline = &pipeline;
    vk::SBT* selected_sbt = &sbt;

    switch(type) {
    case RtxPipelineType::Basic:
        selected_pipeline = &basic_pipeline;
        selected_sbt = &basic_sbt;
        break;
    case RtxPipelineType::Shadow:
        selected_pipeline = &shadow_pipeline;
        selected_sbt = &shadow_sbt;
        break;
    case RtxPipelineType::Path:
        selected_pipeline = &path_pipeline;
        selected_sbt = &path_sbt;
        break;
    }


    selected_pipeline->cmd_bind(cmd_buf);

    descriptor_pool.cmd_bind_set(cmd_buf, 0, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    selected_pipeline->cmd_push_constant(cmd_buf, &rtx_push);

    vk::RtxPipeline::cmd_trace_rays(cmd_buf, *selected_sbt, image_view->image()->extent());
}

void Raytracer::trace(RtxPipelineType type, const RtxPushConstant& rtx_push) {
    vk::CommandBuffer::single_time_submit(vk::QueueType::Compute, [&](vk::ReadyCommandBuffer cmd_buffer)
        { this->cmd_trace(cmd_buffer, type, rtx_push); });
}



Raytracer RaytracerBuilder::build() const {
    Raytracer raytracer{};

    const auto& app_path = utils::AppPath::instance();

    raytracer.pipeline_layout = vk::PipelineLayoutBuilder()
        .add_layout(vk::DescriptorSetLayoutBuilder()
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
                .set_stage_flags(
                    VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                    VK_SHADER_STAGE_MISS_BIT_KHR |
                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
                ))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
                .set_stage_flags(VK_SHADER_STAGE_RAYGEN_BIT_KHR))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                .set_stage_flags(
                    VK_SHADER_STAGE_RAYGEN_BIT_KHR |
                    VK_SHADER_STAGE_MISS_BIT_KHR |
                    VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
                ))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                .set_stage_flags(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                .set_stage_flags(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                .set_stage_flags(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR))
            .add_binding(vk::DescriptorSetLayoutBinding()
                .set_type(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                .set_stage_flags(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR))
            .build())
        .push_constant(vk::PushConstant()
            .add_stage_flag(VK_SHADER_STAGE_RAYGEN_BIT_KHR)
            .add_stage_flag(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
            .add_stage_flag(VK_SHADER_STAGE_MISS_BIT_KHR)
            .set_size(sizeof(RtxPushConstant)))
        .build().to_shared();

    raytracer.camera_uniform_buffer = vk::BufferBuilder()
        .usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
        .queue_types({ vk::QueueType::Compute })
        .memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
        .size(sizeof(CameraUBO))
        .build()
        .to_shared();

    raytracer.descriptor_pool = vk::DescriptorPoolBuilder()
        .pipeline_layout(raytracer.pipeline_layout)
        .add_set(vk::DescriptorSetInfo()
            .set_index(0)
            .set_binding(2, { raytracer.camera_uniform_buffer }))
        .build();

    auto ray_gen_shader = vk::ShaderBuilder().raygen_stage().load_spirv(app_path.get_path("assets/shaders/rtx/ray_gen.spv").string()).build().to_shared();

    auto miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("assets/shaders/rtx/miss.spv").string()).build().to_shared();
    auto shadow_miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("assets/shaders/rtx/shadow_miss.spv").string()).build().to_shared();

    auto basic_closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv(app_path.get_path("assets/shaders/rtx/closest_hit_basic.spv").string()).build().to_shared();
    auto closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv(app_path.get_path("assets/shaders/rtx/closest_hit.spv").string()).build().to_shared();
    auto shadow_closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv(app_path.get_path("assets/shaders/rtx/closest_hit_shadowed.spv").string()).build().to_shared();

    auto path_ray_gen_shader = vk::ShaderBuilder().raygen_stage().load_spirv(app_path.get_path("assets/shaders/pathtracing/ray_gen.spv").string()).build().to_shared();

    auto path_sky_miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("assets/shaders/pathtracing/sky_miss.spv").string()).build().to_shared();
    auto path_sun_miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("assets/shaders/pathtracing/sun_miss.spv").string()).build().to_shared();
    auto path_ambient_miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("assets/shaders/pathtracing/ambient_miss.spv").string()).build().to_shared();
    
    auto path_closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv(app_path.get_path("assets/shaders/pathtracing/closest_hit.spv").string()).build().to_shared();

    vk::ShaderGroup ray_gen_group = vk::ShaderGroup::create_general(ray_gen_shader);

    vk::ShaderGroup miss_group = vk::ShaderGroup::create_general(miss_shader);
    vk::ShaderGroup shadow_miss_group = vk::ShaderGroup::create_general(shadow_miss_shader);

    vk::ShaderGroup basic_hit_group = vk::ShaderGroup::create_hit_closest(basic_closest_hit_shader);
    vk::ShaderGroup hit_group = vk::ShaderGroup::create_hit_closest(closest_hit_shader);
    vk::ShaderGroup shadow_hit_group = vk::ShaderGroup::create_hit_closest(shadow_closest_hit_shader);

    vk::ShaderGroup path_ray_gen_group = vk::ShaderGroup::create_general(path_ray_gen_shader);
    vk::ShaderGroup path_sky_miss_group = vk::ShaderGroup::create_general(path_sky_miss_shader);
    vk::ShaderGroup path_sun_miss_group = vk::ShaderGroup::create_general(path_sun_miss_shader);
    vk::ShaderGroup path_ambient_miss_group = vk::ShaderGroup::create_general(path_ambient_miss_shader);
    vk::ShaderGroup path_hit_group = vk::ShaderGroup::create_hit_closest(path_closest_hit_shader);

    raytracer.basic_pipeline = vk::RtxPipelineBuilder()
        .add_shader_group(ray_gen_group)
        .add_shader_group(miss_group)
        .add_shader_group(basic_hit_group)
        .layout(raytracer.pipeline_layout)
        .max_ray_recursion_depth(1)
        .build();

    raytracer.basic_sbt = raytracer.basic_pipeline.build_sbt(vk::SBTInfo()
        .ray_gen_group(ray_gen_group)
        .miss_groups({ miss_group })
        .hit_groups({ basic_hit_group }));


    raytracer.pipeline = vk::RtxPipelineBuilder()
        .add_shader_group(ray_gen_group)
        .add_shader_group(miss_group)
        .add_shader_group(hit_group)
        .layout(raytracer.pipeline_layout)
        .max_ray_recursion_depth(1)
        .build();

    raytracer.sbt = raytracer.pipeline.build_sbt(vk::SBTInfo()
        .ray_gen_group(ray_gen_group)
        .miss_groups({ miss_group })
        .hit_groups({ hit_group }));


    raytracer.shadow_pipeline = vk::RtxPipelineBuilder()
        .add_shader_group(ray_gen_group)
        .add_shader_group(miss_group)
        .add_shader_group(shadow_miss_group)
        .add_shader_group(shadow_hit_group)
        .layout(raytracer.pipeline_layout)
        .max_ray_recursion_depth(2)
        .build();

    raytracer.shadow_sbt = raytracer.shadow_pipeline.build_sbt(vk::SBTInfo()
        .ray_gen_group(ray_gen_group)
        .miss_groups({ miss_group, shadow_miss_group })
        .hit_groups({ shadow_hit_group }));

    
    raytracer.path_pipeline = vk::RtxPipelineBuilder()
        .add_shader_group(path_ray_gen_group)
        .add_shader_group(path_sky_miss_group)
        .add_shader_group(path_sun_miss_group)
        .add_shader_group(path_ambient_miss_group)
        .add_shader_group(path_hit_group)
        .layout(raytracer.pipeline_layout)
        .max_ray_recursion_depth(_ray_depth)
        .build();

    raytracer.path_sbt = raytracer.path_pipeline.build_sbt(vk::SBTInfo()
        .ray_gen_group(path_ray_gen_group)
        .miss_groups({ path_sky_miss_group, path_sun_miss_group, path_ambient_miss_group })
        .hit_groups({ path_hit_group }));


    return raytracer;
}