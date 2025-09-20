#!/bin/bash

cd "$(dirname "$0")"

./premake/linux/premake5 gmake2
make config=release

./test_rtx_linux -f 9 -s space-station --store-images