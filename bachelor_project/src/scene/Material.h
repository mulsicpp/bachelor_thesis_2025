#pragma once

#include <glm/glm.hpp>

#include "utils/ptr.h"

#include "vk_resources/ImageView.h"
#include "vk_resources/Sampler.h"

#include <cstdint>

struct Texture {
    ptr::Shared<vk::ImageView> image_view{};
    ptr::Shared<vk::Sampler> sampler{};
};

#define MAT_ALPHA_MODE    0x3
#define ALPHA_MODE_OPAQUE 0 
#define ALPHA_MODE_MASK   1 
#define ALPHA_MODE_BLEND  2

#define MAT_DOUBLE_SIDED  0x4

struct Material {
	static const ptr::Shared<Material> default_material;

	alignas(16) glm::vec3 emissive_factor = glm::vec3{ 0.0f, 0.0f, 0.0f };
	float emissive_strength = 1.0f;

	alignas(16) glm::vec4 base_color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	float metallic_factor = 0.0f;
	float roughness_factor = 0.0f;

	float alpha_cutoff = 0.5f;
	uint32_t flags = 0;

	uint32_t base_color_texture = ~uint32_t(0);
	uint32_t metallic_roughness_texture = ~uint32_t(0);
	uint32_t emission_texture = ~uint32_t(0);
	uint32_t normal_texture = ~uint32_t(0);
};