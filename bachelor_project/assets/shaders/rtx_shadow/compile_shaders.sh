cd "$(dirname "$0")"

glslangValidator -V closest_hit.rchit -o closest_hit.spv --target-env spirv1.5

glslangValidator -V shadow_miss.rmiss -o shadow_miss.spv --target-env spirv1.5
glslangValidator -V shadow_hit.rchit -o shadow_hit.spv --target-env spirv1.5