#ifndef RTX_PUSH_GLSL
#define RTX_PUSH_GLSL


#define RTX_PUSH_GREEDY_BIT 0x01

layout(push_constant) uniform RtxPush {
    vec3 light_direction;
    float light_radius;
    vec3 light_color;
    vec3 ambient_color;

    uint sample_factor;
    uint ray_depth;

    uint flags;
} rtx_push;

#endif