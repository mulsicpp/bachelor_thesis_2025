#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rtx_include/rtx_push.glsl"
#include "rtx_include/payload.glsl"

layout(location = 0) rayPayloadInEXT RtxPayload payload;

#include "rtx_include/tlas.glsl"
#include "rtx_include/camera.glsl"

#include "rtx_include/surface.glsl"

#include "rtx_include/rand.glsl"

#ifndef SHADOWS
#define SHADOWS 0
#endif

#ifndef AO
#define AO 0
#endif

#if SHADOWS
layout(location = 1) rayPayloadEXT bool shadow;
#endif

#define OCCLUSION_SAMPLES 16

void main()
{
    SurfacePoint surface = get_surface();

    vec3 normal = surface.normal;

    if(dot(gl_WorldRayDirectionEXT, surface.face_normal) > 0.0f) {
        normal = -normal;
    }

    vec3 light_dir = normalize(rtx_push.light_direction);

    float dot_prod = dot(normal, -light_dir);

#if SHADOWS

    shadow = false;

    vec3 origin = surface.position + surface.normal * 0.00001 * length(surface.position);
    uint  flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

    if(dot_prod > 0.0) {
        shadow = true;

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

    payload.color = vec3(0.0f);

    if(!shadow) {
        payload.color += clamp(dot_prod, 0.0f, 1.0f) * rtx_push.light_color;
    }

#if AO
    float occlusion_factor = 0.0f;

    vec2 seed = vec2(payload.image_coord);

    for(uint i = 0; i < OCCLUSION_SAMPLES; i++) {
        vec3 dir = reflect_diffuse(normal, seed);

        seed += vec2(-seed.y, seed.x) * 0.44676;

        shadow = true;

        traceRayEXT(
            tlas,                      // acceleration structure
            flags,                     // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            1,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            dir,                       // ray direction
            10000.0,                   // maxT
            1                          // location of payload
        );

        if(!shadow) {
            occlusion_factor += 1.0f;
        }
    }

    payload.color += rtx_push.ambient_color * occlusion_factor / OCCLUSION_SAMPLES;
#else
    payload.color += rtx_push.ambient_color;
#endif
#else
    payload.color = rtx_push.ambient_color * 0.5 + clamp(dot_prod, 0.0f, 1.0f) * rtx_push.light_color;
#endif
    payload.color *= surface.color.rgb * 0.7;
}