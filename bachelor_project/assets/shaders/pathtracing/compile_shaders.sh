#!/bin/bash

cd "$(dirname "$0")"

glslangValidator -I.. -V ray_gen.rgen -o ray_gen.spv --target-env spirv1.5

glslangValidator -I.. -V sky.rmiss -o sky_miss.spv --target-env spirv1.5
glslangValidator -I.. -V sun.rmiss -o sun_miss.spv --target-env spirv1.5
glslangValidator -I.. -V ambient.rmiss -o ambient_miss.spv --target-env spirv1.5

glslangValidator -I.. -V closest_hit.rchit -o closest_hit.spv --target-env spirv1.5
glslangValidator -I.. -V any_hit.rahit -o any_hit.spv --target-env spirv1.5