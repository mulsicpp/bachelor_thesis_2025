#pragma once

#include "vk_pipeline/DescriptorPool.h"

class Rasterizer;

class Frame : public utils::Move, public ptr::ToShared<Frame> {
	friend class Rasterizer;
private:
	vk::DescriptorPool descriptor_pool{};
};