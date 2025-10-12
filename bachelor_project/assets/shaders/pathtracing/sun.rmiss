#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rtx_include/rtx_push.glsl"
#include "rtx_include/payload.glsl"

layout(location = 0) rayPayloadInEXT PathPayload payload;

void main()
{
    payload.color = rtx_push.light_color;
}