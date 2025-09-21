cd "$(dirname "$0")"

glslangValidator -V ray_gen.rgen -o ray_gen.spv --target-env spirv1.5
glslangValidator -V miss.rmiss -o miss.spv --target-env spirv1.5
glslangValidator -V shadow_miss.rmiss -o shadow_miss.spv --target-env spirv1.5
glslangValidator -V closest_hit.rchit -o closest_hit.spv --target-env spirv1.5 -DSHADOWS=0
glslangValidator -V closest_hit.rchit -o closest_hit_shadowed.spv --target-env spirv1.5 -DSHADOWS=1