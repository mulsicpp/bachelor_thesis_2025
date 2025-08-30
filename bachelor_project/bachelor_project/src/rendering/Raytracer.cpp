#include "Raytracer.h"

#include "utils/dbg_log.h"

#include "vk_pipeline/Shader.h"



Raytracer RaytracerBuilder::build() const {
    Raytracer raytracer{};

    vk::Shader ray_gen_shader = vk::ShaderBuilder().raygen_stage().load_spirv("assets/shaders/rtx/ray_gen.spv").build();
    vk::Shader closest_hit_shader = vk::ShaderBuilder().closest_hit_stage().load_spirv("assets/shaders/rtx/closest_hit.spv").build();
    vk::Shader miss_shader = vk::ShaderBuilder().miss_stage().load_spirv("assets/shaders/rtx/miss.spv").build();

    dbg_log("loaded all rtx shaders");

    // TODO build raytracer

    return raytracer;
}