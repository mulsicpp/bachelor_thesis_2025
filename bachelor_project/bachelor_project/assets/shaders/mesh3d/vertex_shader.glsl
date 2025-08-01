#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(push_constant) uniform Model {
    mat4 transform;
} model;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = camera.proj * camera.view * model.transform * vec4(inPosition, 1.0);
    fragColor = inPosition + vec3(0.5f);
}