#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rtx_include/rtx_push.glsl"
#include "rtx_include/payload.glsl"

layout(location = 0) rayPayloadInEXT PathPayload payload;

#include "rtx_include/tlas.glsl"
#include "rtx_include/camera.glsl"

#include "rtx_include/surface.glsl"

#include "rtx_include/rand.glsl"

void main()
{
    if(payload.ttl == 0) {
        payload.color = vec3(0.0);
        return;
    }

    SurfacePoint surface = get_surface();
    vec3 origin = surface.position + surface.normal * 0.00001 * length(surface.position);

    vec2 seed = vec2(payload.image_coord) * 12.678 + vec2(payload.ttl) * 43.6;
    vec3 diffuse_dir = reflect_diffuse(surface.normal, seed);

    // dir = normalize(gl_WorldRayDirectionEXT) - 2 * dot(normalize(gl_WorldRayDirectionEXT), normal) * normal;

    if((rtx_push.flags & RTX_PUSH_GREEDY_BIT) != 0) {
        vec3 light_dir = normalize(rtx_push.light_direction);
        vec3 sun_dir = direction_in_cap(-light_dir, rtx_push.light_radius, seed);
        float dot_sun = dot(sun_dir, surface.normal);
        vec3 sun_color = vec3(0.0);

        if(dot_sun > 0.0) {

            uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

            payload.color = sun_color;
            traceRayEXT(
                tlas,                      // acceleration structure
                flags,                     // ray flags
                0xFF,                      // cull mask
                0,                         // sbtRecordOffset
                0,                         // sbtRecordStride
                1,                         // missIndex
                origin,                    // ray origin
                0.0,                       // minT
                sun_dir,                   // ray direction
                10000.0,                   // maxT
                0                          // location of payload
            );

            sun_color = payload.color * dot_sun;
        }

        payload.ttl--;

        traceRayEXT(
            tlas,                      // acceleration structure
            gl_RayFlagsOpaqueEXT,      // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            2,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            diffuse_dir,               // ray direction
            10000.0,                   // maxT
            0                          // location of payload
        );

        payload.color += sun_color;
        payload.color *= 0.7 * surface.color;

    } else {

        payload.ttl--;

        traceRayEXT(
            tlas,                      // acceleration structure
            gl_RayFlagsOpaqueEXT,      // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            0,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            diffuse_dir,               // ray direction
            10000.0,                   // maxT
            0                          // location of payload
        );

        payload.color *= 0.7;
    }
}