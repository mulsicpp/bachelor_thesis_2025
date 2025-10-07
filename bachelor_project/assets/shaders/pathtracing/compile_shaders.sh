cd "$(dirname "$0")"

glslangValidator -V ray_gen.rgen -o ray_gen.spv --target-env spirv1.5

glslangValidator -V sky.rmiss -o sky_miss.spv --target-env spirv1.5
glslangValidator -V sun.rmiss -o sun_miss.spv --target-env spirv1.5
glslangValidator -V ambient.rmiss -o ambient_miss.spv --target-env spirv1.5

glslangValidator -V closest_hit.rchit -o closest_hit.spv --target-env spirv1.5