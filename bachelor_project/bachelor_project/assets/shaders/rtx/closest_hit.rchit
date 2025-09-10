#version 460 core
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

struct Pos {
    float x;
    float y;
    float z;
};

layout(set = 0, binding = 2) uniform Camera {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 3) readonly buffer Indices {
    uint indices[]; 
} indices;

layout(set = 0, binding = 4) readonly buffer Positions {
    Pos positions[];
} positions;

layout(set = 0, binding = 5) readonly buffer DynamicPositions {
    Pos positions[];
} dynamic_positions;

layout(set = 0, binding = 6) readonly buffer Offsets {
    uvec2 offsets[];
} offsets;

hitAttributeEXT vec2 attribs;

const uint OFFSET_VALUE_MASK = 0x7fffffff;
const uint OFFSET_FLAG_MASK = 0x80000000;

void main()
{
    uvec2 offset = offsets.offsets[gl_InstanceCustomIndexEXT + gl_GeometryIndexEXT];

    uint i0, i1, i2;

    if(offset.x == ~0) {
        i0 = gl_PrimitiveID * 3;
        i1 = gl_PrimitiveID * 3 + 1;
        i2 = gl_PrimitiveID * 3 + 2;
    } else {
        i0 = indices.indices[offset.x + gl_PrimitiveID * 3];
        i1 = indices.indices[offset.x + gl_PrimitiveID * 3 + 1];
        i2 = indices.indices[offset.x + gl_PrimitiveID * 3 + 2];
    }

    Pos p0, p1, p2;

    if((offset.y & OFFSET_FLAG_MASK) != 0) {
        offset.y = offset.y & OFFSET_VALUE_MASK;

        p0 = dynamic_positions.positions[offset.y + i0];
        p1 = dynamic_positions.positions[offset.y + i1];
        p2 = dynamic_positions.positions[offset.y + i2];
    } else {
        p0 = positions.positions[offset.y + i0];
        p1 = positions.positions[offset.y + i1];
        p2 = positions.positions[offset.y + i2];
    }

    vec3 v0 = (camera.view * vec4(gl_ObjectToWorldEXT * vec4(p0.x, p0.y, p0.z, 1.0f), 1.0f)).xyz;
    vec3 v1 = (camera.view * vec4(gl_ObjectToWorldEXT * vec4(p1.x, p1.y, p1.z, 1.0f), 1.0f)).xyz;
    vec3 v2 = (camera.view * vec4(gl_ObjectToWorldEXT * vec4(p2.x, p2.y, p2.z, 1.0f), 1.0f)).xyz;

    vec3 normal = normalize(cross(v1 - v0, v2 - v0));

    payload = normal * 0.5f + 0.5f;
}