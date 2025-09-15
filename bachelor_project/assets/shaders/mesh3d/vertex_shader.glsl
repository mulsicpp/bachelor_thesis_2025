#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(push_constant) uniform MeshPush {
    mat4 transform;
    vec4 base_color;
} mesh_push;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = camera.proj * camera.view * mesh_push.transform * vec4(inPosition, 1.0);
    fragColor = mesh_push.base_color.rgb;
}