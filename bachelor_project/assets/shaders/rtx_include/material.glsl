#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

#define MAT_ALPHA_MODE    0x3
#define ALPHA_MODE_OPAQUE 0 
#define ALPHA_MODE_MASK   1 
#define ALPHA_MODE_BLEND  2

#define MAT_DOUBLE_SIDED  0x4

struct Material {
	vec3 emissive_factor;
	float emissive_strength;
	
    vec4 base_color;
	float metallic_factor;
	float roughness_factor;

	float alpha_cutoff;
	uint flags;

	uint base_color_texture;
	uint metallic_roughness_texture;
	uint emission_texture;
	uint normal_texture;
};

#endif