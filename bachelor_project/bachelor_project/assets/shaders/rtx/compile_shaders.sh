cd "$(dirname "$0")"

glslc ray_gen.rgen -o ray_gen.spv -fshader-stage=raygen --target-env=vulkan1.2