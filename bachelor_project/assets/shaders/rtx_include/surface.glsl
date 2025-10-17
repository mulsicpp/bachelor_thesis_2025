#ifndef SURFACE_GLSL
#define SURFACE_GLSL

#extension GL_EXT_nonuniform_qualifier : require

#include "rtx_include/material.glsl"

struct Pos {
    float x;
    float y;
    float z;
};

struct PrimitiveOffset {
	bool dyn;
	uint positions;
	uint normals;
	uint tangents;
	uint uvs;
	uint indices;
    uint material;
};

layout(set = 0, binding = 3) readonly buffer Indices {
    uint indices[]; 
} indices;

layout(set = 0, binding = 4) readonly buffer Positions {
    Pos positions[];
} positions;

layout(set = 0, binding = 5) readonly buffer DynamicPositions {
    Pos positions[];
} dynamic_positions;

layout(set = 0, binding = 6) readonly buffer UVs {
    vec2 uvs[];
} uvs;

layout(set = 0, binding = 7) readonly buffer Offsets {
    PrimitiveOffset offsets[];
} offsets;

layout(set = 0, binding = 8) readonly buffer Materials {
    Material materials[];
} materials;

layout(set = 0, binding = 9) uniform sampler2D textures[];

hitAttributeEXT vec2 attribs;

#define INDEX_UNUSED (~uint(0))

struct SurfacePoint {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec3 face_normal;
};

vec3 get_bary() {
    return vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
}

PrimitiveOffset get_offset() {
    return offsets.offsets[gl_InstanceCustomIndexEXT + gl_GeometryIndexEXT];
}

Material get_material() {
    return materials.materials[get_offset().material];
}

vec4 get_surface_color(Material material, vec2 uv) {
    if(material.base_color_texture != INDEX_UNUSED) {
        return material.base_color * texture(textures[material.base_color_texture], uv);
    } else {
        return material.base_color;
    }
}

uvec3 get_indices() {
    PrimitiveOffset offset = get_offset();
    uint i0, i1, i2;
    if(offset.indices == INDEX_UNUSED) {
        i0 = gl_PrimitiveID * 3;
        i1 = gl_PrimitiveID * 3 + 1;
        i2 = gl_PrimitiveID * 3 + 2;
    } else {
        i0 = indices.indices[offset.indices + gl_PrimitiveID * 3];
        i1 = indices.indices[offset.indices + gl_PrimitiveID * 3 + 1];
        i2 = indices.indices[offset.indices + gl_PrimitiveID * 3 + 2];
    }

    return uvec3(i0, i1, i2);
}

vec2 get_uv(uvec3 i) {
    PrimitiveOffset offset = get_offset();
    if(offset.uvs != INDEX_UNUSED) {
        vec2 uv0, uv1, uv2;

        uv0 = uvs.uvs[offset.uvs + i.x];
        uv1 = uvs.uvs[offset.uvs + i.y];
        uv2 = uvs.uvs[offset.uvs + i.z];

        vec3 bary = get_bary();

        return uv0 * bary.x + uv1 * bary.y + uv2 * bary.z;
    } else {
        return vec2(1.0);
    }
}

SurfacePoint get_surface() {
    vec3 bary = get_bary();

    PrimitiveOffset offset = get_offset();

    uvec3 i = get_indices();

    Pos p0, p1, p2;
    if(offset.dyn) {
        p0 = dynamic_positions.positions[offset.positions + i.x];
        p1 = dynamic_positions.positions[offset.positions + i.y];
        p2 = dynamic_positions.positions[offset.positions + i.z];
    } else {
        p0 = positions.positions[offset.positions + i.x];
        p1 = positions.positions[offset.positions + i.y];
        p2 = positions.positions[offset.positions + i.z];
    }

    vec2 uv = get_uv(i);


    vec3 v0 = gl_ObjectToWorldEXT * vec4(p0.x, p0.y, p0.z, 1.0f);
    vec3 v1 = gl_ObjectToWorldEXT * vec4(p1.x, p1.y, p1.z, 1.0f);
    vec3 v2 = gl_ObjectToWorldEXT * vec4(p2.x, p2.y, p2.z, 1.0f);

    SurfacePoint surface;
    
    surface.face_normal = surface.normal = normalize(cross(v1 - v0, v2 - v0));

    if(offset.normals != INDEX_UNUSED) {
        Pos ln0, ln1, ln2;

        if(offset.dyn) {
            ln0 = dynamic_positions.positions[offset.normals + i.x];
            ln1 = dynamic_positions.positions[offset.normals + i.y];
            ln2 = dynamic_positions.positions[offset.normals + i.z];
        } else {
            ln0 = positions.positions[offset.normals + i.x];
            ln1 = positions.positions[offset.normals + i.y];
            ln2 = positions.positions[offset.normals + i.z];
        }

        vec3 ln =   normalize(vec3(ln0.x, ln0.y, ln0.z)) * bary.x + 
                    normalize(vec3(ln1.x, ln1.y, ln1.z)) * bary.y + 
                    normalize(vec3(ln2.x, ln2.y, ln2.z)) * bary.z;

        surface.normal = normalize(transpose(mat3(gl_WorldToObjectEXT)) * ln);
    } else {
        surface.normal = surface.face_normal;
    }

    Material material = materials.materials[offset.material];

    vec4 base_color_tex = vec4(1.0);

    if(material.base_color_texture != INDEX_UNUSED) {
        base_color_tex = texture(textures[material.base_color_texture], uv);
    }


    surface.position = v0 * bary.x + v1 * bary.y + v2 * bary.z;
    // surface.normal = normalize(cross(v1 - v0, v2 - v0));
    surface.color = material.base_color * base_color_tex;
    // surface.color = vec3(fract(uv), 1.0);

    return surface;
}

#endif