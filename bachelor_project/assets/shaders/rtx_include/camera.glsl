#ifndef CAMERA_GLSL
#define CAMERA_GLSL

layout(set = 0, binding = 2) uniform Camera {
    mat4 view;
    mat4 proj;
    float focus_distance;
    float relative_lens_radius;
} camera;

#endif