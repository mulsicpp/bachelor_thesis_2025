#ifndef PAYLOAD_GLSL
#define PAYLOAD_GLSL

struct PathPayload {
    vec3 color;
    vec2 image_coord;
    uint ttl;
};

struct RtxPayload {
    vec3 color;
    vec2 image_coord;
};

#endif