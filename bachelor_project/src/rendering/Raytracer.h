#pragma once

#include "utils/move.h"
#include "utils/ptr.h"
#include "utils/dbg_log.h"

#include "vk_pipeline/DescriptorPool.h"

#include "vk_rtx/RtxPipeline.h"

#include "scene/Scene.h"
#include "scene/Camera.h"

#define RTX_PUSH_GREEDY 0x01

struct RtxPushConstant {
    alignas(16) glm::vec3 light_direction{ 0.5, -1.0, 0.5 };
    float light_radius{ glm::pi<float>() * 0.00f };
    alignas(16) glm::vec3 light_color{ 1.0, 0.8, 0.5 };
    alignas(16) glm::vec3 ambient_color{ 0.07, 0.15, 0.4 };

    uint32_t sample_factor{ 1 };
    uint32_t ray_depth{ 1 };

    uint32_t flags{ 0 };
};

enum class RtxPipelineType {
    Normal,
    Basic,
    Shadow,
    Path
};


class RaytracerBuilder;

class Raytracer : public utils::Move, public ptr::ToShared<Raytracer> {
    friend class RaytracerBuilder;
private:
    ptr::Shared<vk::PipelineLayout> pipeline_layout{};
    vk::DescriptorPool descriptor_pool{};

    vk::RtxPipeline basic_pipeline{};
    vk::RtxPipeline pipeline{};
    vk::RtxPipeline shadow_pipeline{};
    vk::RtxPipeline path_pipeline{};

    vk::SBT basic_sbt{};
    vk::SBT sbt{};
    vk::SBT shadow_sbt{};
    vk::SBT path_sbt{};

    ptr::Shared<vk::Buffer> camera_uniform_buffer{};
    ptr::Shared<Scene> scene{};
    ptr::Shared<vk::ImageView> image_view{};


public:
    Raytracer() = default;

    void bind_camera(const ptr::Shared<Camera>& camera);
    void bind_scene(const ptr::Shared<Scene>& scene);
    void bind_image(const ptr::Shared<vk::ImageView>& image_view);

    void cmd_trace(vk::ReadyCommandBuffer cmd_buf, RtxPipelineType type, const RtxPushConstant& rtx_push = {});
    void trace(RtxPipelineType type, const RtxPushConstant& rtx_push = {});
};


class RaytracerBuilder {
public:
    using Ref = RaytracerBuilder&;

private:
    // TODO raytracer builder parameters
    uint32_t _ray_depth{ 1 };

public:
    RaytracerBuilder() = default;

    inline Ref ray_depth(uint32_t ray_depth) { _ray_depth = ray_depth; return *this; }

    Raytracer build() const;
};