#ifndef SURFACE_GLSL
#define SURFACE_GLSL

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

hitAttributeEXT vec2 attribs;

#define OFFSET_ATTR_UNUSED (~uint(0))

struct SurfacePoint {
    vec3 position;
    vec3 normal;
    vec3 color;
};

SurfacePoint get_surface() {
    vec3 bary = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

    PrimitiveOffset offset = offsets.offsets[gl_InstanceCustomIndexEXT + gl_GeometryIndexEXT];

    uint i0, i1, i2;
    if(offset.indices == OFFSET_ATTR_UNUSED) {
        i0 = gl_PrimitiveID * 3;
        i1 = gl_PrimitiveID * 3 + 1;
        i2 = gl_PrimitiveID * 3 + 2;
    } else {
        i0 = indices.indices[offset.indices + gl_PrimitiveID * 3];
        i1 = indices.indices[offset.indices + gl_PrimitiveID * 3 + 1];
        i2 = indices.indices[offset.indices + gl_PrimitiveID * 3 + 2];
    }

    Pos p0, p1, p2;
    if(offset.dyn) {
        p0 = dynamic_positions.positions[offset.positions + i0];
        p1 = dynamic_positions.positions[offset.positions + i1];
        p2 = dynamic_positions.positions[offset.positions + i2];
    } else {
        p0 = positions.positions[offset.positions + i0];
        p1 = positions.positions[offset.positions + i1];
        p2 = positions.positions[offset.positions + i2];
    }

    vec2 uv;

    if(offset.uvs != OFFSET_ATTR_UNUSED) {
        vec2 uv0, uv1, uv2;

        uv0 = uvs.uvs[offset.uvs + i0];
        uv1 = uvs.uvs[offset.uvs + i1];
        uv2 = uvs.uvs[offset.uvs + i2];

        uv = uv0 * bary.x + uv1 * bary.y + uv2 * bary.z;
    } else {
        uv = vec2(1.0);
    }


    vec3 v0 = gl_ObjectToWorldEXT * vec4(p0.x, p0.y, p0.z, 1.0f);
    vec3 v1 = gl_ObjectToWorldEXT * vec4(p1.x, p1.y, p1.z, 1.0f);
    vec3 v2 = gl_ObjectToWorldEXT * vec4(p2.x, p2.y, p2.z, 1.0f);

    SurfacePoint surface;
    

    if(offset.normals != OFFSET_ATTR_UNUSED && false) {
        Pos ln0, ln1, ln2;

        if(offset.dyn) {
            ln0 = dynamic_positions.positions[offset.normals + i0];
            ln1 = dynamic_positions.positions[offset.normals + i1];
            ln2 = dynamic_positions.positions[offset.normals + i2];
        } else {
            ln0 = positions.positions[offset.normals + i0];
            ln1 = positions.positions[offset.normals + i1];
            ln2 = positions.positions[offset.normals + i2];
        }

        vec3 ln =   normalize(vec3(ln0.x, ln0.y, ln0.z)) * bary.x + 
                    normalize(vec3(ln1.x, ln1.y, ln1.z)) * bary.y + 
                    normalize(vec3(ln2.x, ln2.y, ln2.z)) * bary.z;

        surface.normal = normalize(transpose(mat3(gl_WorldToObjectEXT)) * ln);
    } else {
        surface.normal = normalize(cross(v1 - v0, v2 - v0));
    }


    surface.position = v0 * bary.x + v1 * bary.y + v2 * bary.z;
    // surface.normal = normalize(cross(v1 - v0, v2 - v0));
    surface.color = vec3(1.0);

    return surface;
}

#endif