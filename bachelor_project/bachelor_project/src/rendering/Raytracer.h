#pragma once

#include "utils/move.h"
#include "utils/ptr.h"
#include "utils/dbg_log.h"

#include "vk_pipeline/DescriptorPool.h"

#include "vk_rtx/RtxPipeline.h"

#include "scene/Scene.h"
#include "scene/Camera.h"

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

    inline void bind_camera(const ptr::Shared<Camera>& camera) {
		//*camera_uniform_buffer->mapped_data<CameraUBO>() = camera->as_camera_ubo();
		//camera_uniform_buffer->flush();
	}

	inline void bind_scene(const ptr::Shared<Scene>& scene) {
        this->scene = scene;
        descriptor_pool.update_set_binding(0, 0, vk::TlasDescriptorInfo(scene->get_tlas()));
    }
    inline void bind_image(const ptr::Shared<vk::ImageView>& image_view) { 
        this->image_view = image_view;
        descriptor_pool.update_set_binding(0, 1, vk::ImageDescriptorInfo(image_view));
    }

	void cmd_draw(vk::ReadyCommandBuffer cmd_buf);
    void draw();
};

class RaytracerBuilder {
public:
    using Ref = RaytracerBuilder&;

private:
    // TODO raytracer builder parameters

public:
    RaytracerBuilder() = default;

    Raytracer build() const;
};