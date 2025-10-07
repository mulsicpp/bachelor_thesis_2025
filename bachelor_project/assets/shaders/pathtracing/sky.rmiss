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