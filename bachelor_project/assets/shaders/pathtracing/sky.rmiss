#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rtx_include/rtx_push.glsl"
#include "rtx_include/payload.glsl"

layout(location = 0) rayPayloadInEXT PathPayload payload;

vec3 sky_color(vec3 dir) {

    float cos_r = cos(rtx_push.light_radius);
    vec3 light_dir = normalize(rtx_push.light_direction);

    if(dot(dir, -light_dir) >= cos_r) {
        return rtx_push.ambient_color + rtx_push.light_color * 0.5 / (1.0f - cos_r);
    }
    return rtx_push.ambient_color;
}

void main()
{
    payload.color = sky_color(normalize(gl_WorldRayDirectionEXT));
}