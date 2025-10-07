#version 460 core
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT struct Payload {
    vec3 color;
    vec2 image_coord;
} payload;

void main()
{
    payload.color = vec3(0.0);
}