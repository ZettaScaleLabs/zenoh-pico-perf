#!/usr/bin/env bash

root_dir=$(git rev-parse --show-toplevel)
install_dir=$root_dir/install
mkdir -p $install_dir
cd $install_dir
mkdir -p src

# git -C zenoh-pico pull 2> /dev/null || git clone https://github.com/eclipse-zenoh/zenoh-pico
git -C ./src/zenoh-pico pull 2> /dev/null || git clone https://github.com/cguimaraes/zenoh-pico -b memory-checks ./src/zenoh-pico

cd ./src/zenoh-pico
cmake -Bbuild \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$root_dir/install
cmake --build build --target install -j
