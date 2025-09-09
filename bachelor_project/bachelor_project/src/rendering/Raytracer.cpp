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

void Raytracer::cmd_draw(vk::ReadyCommandBuffer cmd_buf) {
    pipeline.cmd_bind(cmd_buf);

    descriptor_pool.cmd_bind_set(cmd_buf, 0, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);

    vk::RtxPipeline::cmd_trace_rays(cmd_buf, sbt, image_view->image()->extent());
}

void Raytracer::draw() {
    vk::CommandBuffer::single_time_submit(vk::QueueType::Compute, [&](vk::ReadyCommandBuffer cmd_buffer)
        { this->cmd_draw(cmd_buffer); });
}



Raytracer RaytracerBuilder::build() const {
    Raytracer raytracer{};

    const auto& app_path = utils::AppPath::instance();

    auto ray_gen_shader = vk::ShaderBuilder().raygen_stage().load_spirv(app_path.get_path("../../assets/shaders/rtx/ray_gen.spv")).build().to_shared();
    auto closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv(app_path.get_path("../../assets/shaders/rtx/closest_hit.spv")).build().to_shared();
    auto miss_shader = vk::ShaderBuilder().miss_stage().load_spirv(app_path.get_path("../../assets/shaders/rtx/miss.spv")).build().to_shared();
    dbg_log("loaded all rtx shaders");

    vk::ShaderGroup ray_gen_group = vk::ShaderGroup::create_general(ray_gen_shader);
    vk::ShaderGroup miss_group = vk::ShaderGroup::create_general(miss_shader);
    vk::ShaderGroup hit_group = vk::ShaderGroup::create_hit_closest(closest_hit_shader);
    dbg_log("created shader groups");

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
        .build().to_shared();
    dbg_log("created rtx pipeline layout");


    // TODO build raytracer
    raytracer.pipeline = vk::RtxPipelineBuilder()
        .add_shader_group(ray_gen_group)
        .add_shader_group(miss_group)
        .add_shader_group(hit_group)
        .layout(raytracer.pipeline_layout)
        .max_ray_recursion_depth(1)
        .build();
    dbg_log("created rtx pipeline");

    raytracer.sbt = raytracer.pipeline.build_sbt(vk::SBTInfo()
        .ray_gen_group(ray_gen_group)
        .miss_groups({ miss_group })
        .hit_groups({ hit_group }));
    dbg_log("built sbt");

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

    return raytracer;
}