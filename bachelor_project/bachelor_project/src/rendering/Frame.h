#pragma once

#include "vk_pipeline/DescriptorPool.h"

#include "scene/Camera.h"
#include "scene/Node.h"
#include "scene/Scene.h"

class Rasterizer;

class Frame : public utils::Move, public ptr::ToShared<Frame> {
	friend class Rasterizer;
private:
	ptr::Shared<vk::Buffer> camera_uniform_buffer;
	vk::DescriptorPool descriptor_pool{};

public:
	CameraUBO* p_camera_ubo;
	ptr::Shared<Scene> scene;
};