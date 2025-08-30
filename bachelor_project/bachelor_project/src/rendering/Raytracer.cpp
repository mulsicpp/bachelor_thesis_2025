#include "Raytracer.h"

#include "utils/dbg_log.h"

#include "vk_pipeline/Shader.h"



Raytracer RaytracerBuilder::build() const {
    Raytracer raytracer{};

    vk::Shader ray_gen_shader = vk::ShaderBuilder().raygen_stage().load_spirv("assets/shaders/rtx/ray_gen.spv").build();
    vk::Shader closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv("assets/shaders/rtx/closest_hit.spv").build();
    vk::Shader miss_shader = vk::ShaderBuilder().miss_stage().load_spirv("assets/shaders/rtx/miss.spv").build();

    dbg_log("loaded all rtx shaders");

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
            .build())
        .build().to_shared();
    dbg_log("created rtx pipeline layout");


    // TODO build raytracer
    raytracer.pipeline = vk::RtxPipelineBuilder()
        .layout(raytracer.pipeline_layout)
        .build();

    return raytracer;
}