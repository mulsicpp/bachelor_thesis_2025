#!/bin/bash

cd "$(dirname "$0")"

./premake/linux/premake5 gmake2
make

cd bachelor_project
./bin/linux-Debug/bachelor_project_linux "assets/scenes/BrainStem/glTF/BrainStem.gltf" -f 30