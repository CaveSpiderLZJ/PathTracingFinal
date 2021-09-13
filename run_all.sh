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
# bin/PA1 testcases/spheres.txt output/spheres.bmp
# bin/PA1 testcases/bunny.txt output/bunny.bmp
# bin/PA1 testcases/dragon.txt output/dragon.bmp
# bin/PA1 testcases/room.txt output/room.bmp
# bin/PA1 testcases/earth.txt output/earth.bmp
# bin/PA1 testcases/drop.txt output/drop.bmp
# bin/PA1 testcases/dragon.txt output/dragon.bmp
# bin/PA1 testcases/depth.txt output/depth.bmp
# bin/PA1 testcases/porcelain.txt output/porcelain.bmp
# bin/PA1 testcases/texture.txt output/texture.bmp
# bin/PA1 testcases/threebody.txt output/threebody.bmp
# bin/PA1 testcases/wineglass.txt output/wineglass.bmp
