#version 460 core
#extension GL_EXT_ray_tracing : require

layout(push_constant) uniform RtxPush {
    vec3 light_direction;
    float light_radius;
    vec3 light_color;
    vec3 ambient_color;

    uint sample_factor;
    uint ray_depth;
} rtx_push;

struct Payload {
    vec3 color;
    vec2 image_coord;
    uint ttl;
};

layout(location = 0) rayPayloadInEXT Payload payload;

void main()
{
    payload.color = rtx_push.ambient_color;
}