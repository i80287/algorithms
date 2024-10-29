#!/usr/bin/env bash

set -e

cmake_build_dir=cmake-build-dir

OLDIFS=$IFS
IFS=','
for cc_and_cxx in clang,clang++ gcc,g++ i686-w64-mingw32-gcc-posix,i686-w64-mingw32-g++-posix x86_64-w64-mingw32-gcc-posix,x86_64-w64-mingw32-g++-posix; do
    if [ -d "$cmake_build_dir" ]; then rm -r "$cmake_build_dir"; fi
    mkdir -p "$cmake_build_dir"
    set -- $cc_and_cxx
    echo "cc = $1, c++ = $2"

    cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER="$1" \
        -D CMAKE_CXX_COMPILER="$2" \
        -S . \
        -B $cmake_build_dir
    cmake --build ./$cmake_build_dir --parallel "$(nproc)"
done
IFS=$OLDIFS
