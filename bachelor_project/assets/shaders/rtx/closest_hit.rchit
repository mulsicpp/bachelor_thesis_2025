#version 460 core
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

struct Pos {
    float x;
    float y;
    float z;
};

layout(push_constant) uniform RtxPush {
    vec3 light_direction;
    vec3 light_color;
    vec3 ambient_color;

    uint sample_factor;
} rtx_push;

layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

layout(set = 0, binding = 2) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 3) readonly buffer Indices {
    uint indices[]; 
} indices;

layout(set = 0, binding = 4) readonly buffer Positions {
    Pos positions[];
} positions;

layout(set = 0, binding = 5) readonly buffer DynamicPositions {
    Pos positions[];
} dynamic_positions;

layout(set = 0, binding = 6) readonly buffer Offsets {
    uvec2 offsets[];
} offsets;

hitAttributeEXT vec2 attribs;

const uint OFFSET_VALUE_MASK = 0x7fffffff;
const uint OFFSET_FLAG_MASK = 0x80000000;

#ifndef SHADOWS
#define SHADOWS 0
#endif

#if SHADOWS
layout(location = 1) rayPayloadEXT bool shadow;
#endif

void main()
{
    uvec2 offset = offsets.offsets[gl_InstanceCustomIndexEXT + gl_GeometryIndexEXT];

    uint i0, i1, i2;

    if(offset.x == ~0) {
        i0 = gl_PrimitiveID * 3;
        i1 = gl_PrimitiveID * 3 + 1;
        i2 = gl_PrimitiveID * 3 + 2;
    } else {
        i0 = indices.indices[offset.x + gl_PrimitiveID * 3];
        i1 = indices.indices[offset.x + gl_PrimitiveID * 3 + 1];
        i2 = indices.indices[offset.x + gl_PrimitiveID * 3 + 2];
    }

    Pos p0, p1, p2;

    if((offset.y & OFFSET_FLAG_MASK) != 0) {
        offset.y = offset.y & OFFSET_VALUE_MASK;

        p0 = dynamic_positions.positions[offset.y + i0];
        p1 = dynamic_positions.positions[offset.y + i1];
        p2 = dynamic_positions.positions[offset.y + i2];
    } else {
        p0 = positions.positions[offset.y + i0];
        p1 = positions.positions[offset.y + i1];
        p2 = positions.positions[offset.y + i2];
    }

    vec3 v0 = gl_ObjectToWorldEXT * vec4(p0.x, p0.y, p0.z, 1.0f);
    vec3 v1 = gl_ObjectToWorldEXT * vec4(p1.x, p1.y, p1.z, 1.0f);
    vec3 v2 = gl_ObjectToWorldEXT * vec4(p2.x, p2.y, p2.z, 1.0f);

    vec3 normal = normalize(cross(v1 - v0, v2 - v0));

    vec3 light_dir = normalize(rtx_push.light_direction);

    float dot_prod = dot(normal, -light_dir);

#if SHADOWS

    shadow = false;

    if(dot_prod > 0.0) {

        shadow = true;
        vec3  origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT + normal * 0.00001 * gl_HitTEXT;
        uint  flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

        traceRayEXT(
            tlas,                      // acceleration structure
            flags,                     // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            1,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            -light_dir,                // ray direction
            10000.0,                   // maxT
            1                          // location of payload
        );
    }

    payload = rtx_push.ambient_color;

    if(!shadow) {
        payload += clamp(dot_prod, 0.0f, 1.0f) * rtx_push.light_color;
    }
#else
    payload = rtx_push.ambient_color + clamp(dot_prod, 0.0f, 1.0f) * rtx_push.light_color;
#endif
}