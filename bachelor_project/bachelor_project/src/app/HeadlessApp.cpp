#include "HeadlessApp.h"

#include "utils/defines.h"
#include "utils/AppPath.h"

#include <algorithm>

#include "vk_core/format.h"

#include "scene/Scene.h"

HeadlessApp::HeadlessApp()
{
    auto context_info = vk::ContextInfo()
        .app_name(APP_NAME)
        .use_raytracing(true);

    vk::Context::create(context_info);

    color_image = vk::ImageBuilder()
        .extent({ IMAGE_WIDTH, IMAGE_HEIGHT })
        .format(VK_FORMAT_R8G8B8A8_UNORM)
        .queue_types({ vk::QueueType::Transfer, vk::QueueType::Graphics })
        .usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        .aspect(VK_IMAGE_ASPECT_COLOR_BIT)
        .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
        .tiling(VK_IMAGE_TILING_OPTIMAL)
        .build()
        .to_shared();

    depth_image = vk::ImageBuilder()
        .extent({ IMAGE_WIDTH, IMAGE_HEIGHT })
        .format(vk::find_depth_format())
        .queue_types({ vk::QueueType::Transfer, vk::QueueType::Graphics })
        .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        .aspect(VK_IMAGE_ASPECT_DEPTH_BIT)
        .memory_usage(VMA_MEMORY_USAGE_GPU_ONLY)
        .tiling(VK_IMAGE_TILING_OPTIMAL)
        .build()
        .to_shared();

    auto color_attachment = vk::Attachment()
        .set_type(vk::AttachmentType::Color)
        .set_format(color_image->format())
        .set_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR)
        .set_store_op(VK_ATTACHMENT_STORE_OP_STORE)
        .set_final_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    auto depth_attachment = vk::Attachment()
        .set_type(vk::AttachmentType::Depth)
        .set_format(depth_image->format())
        .set_load_op(VK_ATTACHMENT_LOAD_OP_CLEAR)
        .set_store_op(VK_ATTACHMENT_STORE_OP_DONT_CARE)
        .set_final_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    rasterizer = RasterizerBuilder()
        .color_attachment(color_attachment)
        .depth_attachment(depth_attachment)
        .build()
        .to_shared();

    framebuffer = vk::FramebufferBuilder()
        .render_pass(rasterizer->get_render_pass())
        .add_image(color_image)
        .add_image(depth_image)
        .build();

    skinner = SkinnerBuilder().build();

    camera = ptr::make_shared<AppCamera>();

    camera->aspect = ((float)IMAGE_WIDTH) / ((float)IMAGE_HEIGHT);
    camera->theta = glm::pi<float>() * 0.75f;
    camera->phi = glm::pi<float>() * 0.25f;
    camera->center = glm::vec3{ 0.0f, -1.0f, 0.0f };
    camera->distance /= 2;

    scene = ptr::make_shared<Scene>(Scene::load(utils::AppPath::instance().get_path("../../assets/scenes/BrainStem/glTF/BrainStem.gltf").string()));
    // scene = ptr::make_shared<Scene>(Scene::load("C:/Users/chris/projects/models/glTF-Sample-Models/2.0/Fox/glTF/Fox.gltf"));

    rasterizer->bind_camera(camera);
    rasterizer->bind_scene(scene);

    skinner.bind_scene(scene);
}

void HeadlessApp::run()
{
    auto& animation = scene->get_animation(0);

    for (uint32_t i = 0; i < 5; i++)
    {
        printf("drawing frame: %u\n", i);
        animation.apply_for(i * 0.1f);
        scene->update_transforms();
        skinner.skin_scene();

        rasterizer->draw(&framebuffer);
        color_image->store_in_file("render_result_" + std::to_string(i) +  ".png");
    }

    vk::Context::get()->wait_device_idle();
}