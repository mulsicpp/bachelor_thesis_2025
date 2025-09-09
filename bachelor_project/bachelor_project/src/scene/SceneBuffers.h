#pragma once

#include "utils/ptr.h"

#include "vk_resources/Buffer.h"

struct SceneBuffers {
    ptr::Shared<vk::Buffer> positions{};
    ptr::Shared<vk::Buffer> indices{};
    ptr::Shared<vk::Buffer> dynamic_positions{};

    ptr::Shared<vk::Buffer> primitive_offsets{};
};