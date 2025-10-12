#!/bin/bash

cd "$(dirname "$0")"

glslc -fshader-stage=vert vertex_shader.glsl -o vert.spv
glslc -fshader-stage=frag fragment_shader.glsl -o frag.spv