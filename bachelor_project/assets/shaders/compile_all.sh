#!/bin/bash

cd "$(dirname "$0")"

./mesh3d/compile_shaders.sh
./rtx/compile_shaders.sh
./pathtracing/compile_shaders.sh