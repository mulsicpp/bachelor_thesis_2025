#!/bin/bash

cd "$(dirname "$0")"

./premake/linux/premake5 gmake2
make config=release

./bachelor_project_linux -f 9 -s brainstem --store-images