#!/usr/bin/env bash

set -e

cmake_build_dir=cmake-build-dir

OLDIFS=$IFS
IFS=','
for cc_and_cxx in \
    clang-18,clang++-18 clang-19,clang++-19 clang-20,clang++-20 \
    gcc-13,g++-13 gcc-14,g++-14 \
    i686-w64-mingw32-gcc-posix,i686-w64-mingw32-g++-posix \
    x86_64-w64-mingw32-gcc-posix,x86_64-w64-mingw32-g++-posix \
; do
    if [ -d "$cmake_build_dir" ]; then rm -r "$cmake_build_dir"; fi
    mkdir -p "$cmake_build_dir"
    set -- $cc_and_cxx
    c_compiler=$1
    cxx_compiler=$2
    for force_disable_checks in ON OFF; do
        echo "cc = $c_compiler, c++ = $cxx_compiler, force_disable_checks = $force_disable_checks"
        cmake \
            -G Ninja \
            -D CMAKE_BUILD_TYPE=RelWithDebInfo \
            -D CMAKE_C_COMPILER="$c_compiler" \
            -D CMAKE_CXX_COMPILER="$cxx_compiler" \
            -D UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS=$force_disable_checks \
            -S . \
            -B $cmake_build_dir
        cmake --build ./$cmake_build_dir --parallel
    done
done
IFS=$OLDIFS
