#!/usr/bin/env bash

# If project not ready, generate cmake file.
if [[ ! -d build ]]; then
    mkdir -p build
    cd build
    cmake ..
    cd ..
else
    rm -r build
    mkdir -p build
    cd build
    cmake ..
    cd ..
fi

# Build project.
cd build
make -j
cd ..

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.
mkdir -p output
# bin/PA1 testcases/scene01_basic.txt output/scene01.bmp
# bin/PA1 testcases/scene02_cube.txt output/scene02.bmp
# bin/PA1 testcases/scene03_sphere.txt output/scene03.bmp
# bin/PA1 testcases/scene04_axes.txt output/scene04.bmp
# bin/PA1 testcases/scene05_bunny_200.txt output/scene05.bmp
# bin/PA1 testcases/scene06_bunny_1k.txt output/scene06.bmp
# bin/PA1 testcases/scene07_shine.txt output/scene07.bmp
# bin/PA1 testcases/spheres.txt output/spheres.bmp
# bin/PA1 testcases/bunny.txt output/bunny.bmp
# bin/PA1 testcases/dragon.txt output/dragon.bmp
# bin/PA1 testcases/room.txt output/room.bmp
# bin/PA1 testcases/earth.txt output/earth.bmp
bin/PA1 testcases/drop.txt output/drop.bmp
# bin/PA1 testcases/dragon.txt output/dragon.bmp
# bin/PA1 testcases/depth.txt output/depth.bmp
# bin/PA1 testcases/porcelain.txt output/porcelain.bmp
# bin/PA1 testcases/texture.txt output/texture.bmp
