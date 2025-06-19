#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 3) in vec3 instanceColor;
layout(location = 4) in vec3 offset;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ((ubo.model * vec4(inPosition, 1.0)) + vec4(offset, 0.0));
    //fragColor = inColor;
    //fragColor = vec3(0.0f, inTexCoord);
    fragColor = instanceColor;
    fragTexCoord = inTexCoord;
}