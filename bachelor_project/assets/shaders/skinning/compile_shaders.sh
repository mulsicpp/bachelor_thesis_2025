cd "$(dirname "$0")"

glslc -fshader-stage=vert vertex_shader.glsl -o vert.spv