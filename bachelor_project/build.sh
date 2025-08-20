!#/bin/bash

cd "$(dirname "$0")"

./premake/linux/premake5 gmake2
make
