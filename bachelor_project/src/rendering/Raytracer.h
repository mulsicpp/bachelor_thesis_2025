#pragma once

#include "utils/move.h"
#include "utils/ptr.h"
#include "utils/dbg_log.h"

#include "vk_pipeline/DescriptorPool.h"

#include "vk_rtx/RtxPipeline.h"

#include "scene/Scene.h"
#include "scene/Camera.h"

struct RtxPushConstant {
    alignas(16) glm::vec3 light_direction{0.5, -1.0, 0.5};
    alignas(16) glm::vec3 light_color{0.9, 0.9, 0.8};
    alignas(16) glm::vec3 ambient_color{0.1, 0.1, 0.2};

    uint32_t sample_factor{1};
};


class RaytracerBuilder;

class Raytracer : public utils::Move, public ptr::ToShared<Raytracer> {
    friend class RaytracerBuilder;
private:
    ptr::Shared<vk::PipelineLayout> pipeline_layout{};
    vk::RtxPipeline pipeline{};
    vk::SBT sbt{};

    ptr::Shared<vk::Buffer> camera_uniform_buffer{};
	ptr::Shared<Scene> scene{};
    ptr::Shared<vk::ImageView> image_view{};

	vk::DescriptorPool descriptor_pool{};

public:
    Raytracer() = default;

    void bind_camera(const ptr::Shared<Camera>& camera);
	void bind_scene(const ptr::Shared<Scene>& scene);
    void bind_image(const ptr::Shared<vk::ImageView>& image_view);

	void cmd_draw(vk::ReadyCommandBuffer cmd_buf, const RtxPushConstant& rtx_push = {});
    void draw(const RtxPushConstant& rtx_push = {});
};


class RaytracerBuilder {
public:
    using Ref = RaytracerBuilder&;

private:
    // TODO raytracer builder parameters
    bool _shadows{ false };

public:
    RaytracerBuilder() = default;

    inline Ref shadows(bool shadows) { _shadows = shadows; return *this; }

    Raytracer build() const;
};