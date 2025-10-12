#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

#include "rtx_include/payload.glsl"

layout(location = 0) rayPayloadInEXT RtxPayload payload;

void main()
{
    payload.color = vec3(0.0);
}