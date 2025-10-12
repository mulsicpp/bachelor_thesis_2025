#ifndef RAND_GLSL
#define RAND_GLSL

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) + sin(dot(co.yx, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand_better(vec2 co) {
    mat2 transform = mat2(vec2(3.0, -0.5), vec2(0.4, 3.4));
    return fract(rand(co) + rand(transform * co));
}

vec3 reflect_diffuse(vec3 normal, vec2 seed) {
    vec3 tangent = normalize(abs(normal.x) > 0.9 ? vec3(0,1,0) : vec3(1,0,0));
    vec3 bitangent = normalize(cross(normal, tangent));
    tangent = cross(bitangent, normal);

    float u1 = rand_better(seed);
    float u2 = rand_better(seed + 0.37);
    
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
    float r1 = rand_better(seed + 2.5351);
    float r2 = rand_better(seed + 0.1234);

    float z = r1 * (1.0 - cos_theta) + cos_theta;
    float phi = 2.0 * 3.141592653589793 * r2;
    float s = sqrt(max(0.0, 1.0 - z*z)); // sin(theta)

    vec3 tangent = normalize(abs(dir.x) > 0.9 ? vec3(0,1,0) : vec3(1,0,0));
    vec3 bitangent = normalize(cross(dir, tangent));
    tangent = cross(bitangent, dir);

    return normalize(s * (cos(phi) * tangent + sin(phi) * bitangent) + z * dir);
}

#endif