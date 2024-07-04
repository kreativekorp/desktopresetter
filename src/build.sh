#!/usr/bin/env bash
mkdir -p ../build
cd ../build
cmake ../src -DCMAKE_TOOLCHAIN_FILE=/home/beckie/Software/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
make
