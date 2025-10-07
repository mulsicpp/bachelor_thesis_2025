#version 460 core
#extension GL_EXT_ray_tracing : require

#define RTX_PUSH_GREEDY 0x01

layout(push_constant) uniform RtxPush {
    vec3 light_direction;
    float light_radius;
    vec3 light_color;
    vec3 ambient_color;

    uint sample_factor;
    uint ray_depth;

    uint flags;
} rtx_push;

struct Payload {
    vec3 color;
    vec2 image_coord;
    uint ttl;
};

layout(location = 0) rayPayloadInEXT Payload payload;

struct Pos {
    float x;
    float y;
    float z;
};

layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;

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

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * sin(dot(co.yx, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 reflect_diffuse(vec3 normal, vec2 seed) {
    vec3 tangent = normalize(abs(normal.x) > 0.9 ? vec3(0,1,0) : vec3(1,0,0));
    vec3 bitangent = normalize(cross(normal, tangent));
    tangent = cross(bitangent, normal);

    float u1 = fract(rand(seed) + rand(seed * 4.33));
    float u2 = rand(seed + 0.37);
    
    float r = sqrt(u1);
    float theta = 2.0 * 3.14159265 * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0 - u1);
    
    return normalize(x * tangent + y * bitangent + z * normal);
}

vec3 direction_in_cap(vec3 dir, float theta, vec2 seed){
    dir = normalize(dir);
    float cos_theta = cos(theta);

    // two randoms
    float r1 = rand(seed + 2.5351);
    float r2 = rand(seed + 0.1234);

    float z = r1 * (1.0 - cos_theta) + cos_theta;
    float phi = 2.0 * 3.141592653589793 * r2;
    float s = sqrt(max(0.0, 1.0 - z*z)); // sin(theta)

    vec3 tangent = normalize(abs(dir.x) > 0.9 ? vec3(0,1,0) : vec3(1,0,0));
    vec3 bitangent = normalize(cross(dir, tangent));
    tangent = cross(bitangent, dir);

    return normalize(s * (cos(phi) * tangent + sin(phi) * bitangent) + z * dir);
}

#define GREEDY 0

void main()
{
    if(payload.ttl == 0) {
        payload.color = vec3(0.0);
        return;
    }

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

    vec3 v0 = gl_ObjectToWorldEXT * vec4(p0.x, p0.y, p0.z, 1.0f);
    vec3 v1 = gl_ObjectToWorldEXT * vec4(p1.x, p1.y, p1.z, 1.0f);
    vec3 v2 = gl_ObjectToWorldEXT * vec4(p2.x, p2.y, p2.z, 1.0f);

    vec3 normal = normalize(cross(v1 - v0, v2 - v0));
    vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT + normal * 0.00001 * gl_HitTEXT;

    vec2 seed = vec2(payload.image_coord) * 142.678 + vec2(payload.ttl) * 43.6;
    vec3 diffuse_dir = reflect_diffuse(normal, seed);

    // dir = normalize(gl_WorldRayDirectionEXT) - 2 * dot(normalize(gl_WorldRayDirectionEXT), normal) * normal;

    if((rtx_push.flags & RTX_PUSH_GREEDY) != 0) {

        vec3 light_dir = normalize(rtx_push.light_direction);

        vec3 sun_dir = direction_in_cap(-light_dir, rtx_push.light_radius, seed);

        float dot_sun = dot(sun_dir, normal);

        vec3 sun_color = vec3(0.0);

        if(dot_sun > 0.0) {

            uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;

            payload.color = sun_color;
            traceRayEXT(
                tlas,                      // acceleration structure
                flags,                     // ray flags
                0xFF,                      // cull mask
                0,                         // sbtRecordOffset
                0,                         // sbtRecordStride
                1,                         // missIndex
                origin,                    // ray origin
                0.0,                       // minT
                sun_dir,                   // ray direction
                10000.0,                   // maxT
                0                          // location of payload
            );

            sun_color = payload.color * dot_sun;
        }

        payload.ttl--;

        traceRayEXT(
            tlas,                      // acceleration structure
            gl_RayFlagsOpaqueEXT,      // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            2,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            diffuse_dir,               // ray direction
            10000.0,                   // maxT
            0                          // location of payload
        );

        payload.color += sun_color;
        payload.color *= 0.7;

    } else {

        payload.ttl--;

        traceRayEXT(
            tlas,                      // acceleration structure
            gl_RayFlagsOpaqueEXT,      // ray flags
            0xFF,                      // cull mask
            0,                         // sbtRecordOffset
            0,                         // sbtRecordStride
            0,                         // missIndex
            origin,                    // ray origin
            0.0,                       // minT
            diffuse_dir,               // ray direction
            10000.0,                   // maxT
            0                          // location of payload
        );

        payload.color *= 0.7;
    }
}