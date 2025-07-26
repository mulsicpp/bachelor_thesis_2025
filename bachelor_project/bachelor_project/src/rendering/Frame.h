#pragma once

#include "vk_pipeline/DescriptorPool.h"

class Rasterizer;

class Frame : public utils::Move, public ptr::ToShared<Frame> {
	friend class Rasterizer;
private:
	ptr::Shared<vk::Buffer> camera_uniform_buffer;
	ptr::Shared<vk::Buffer> model_uniform_buffer;
	vk::DescriptorPool descriptor_pool{};
};