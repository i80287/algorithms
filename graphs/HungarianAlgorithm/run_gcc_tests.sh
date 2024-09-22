#!/bin/bash

build_dir=cmake-build-tests-gcc

set -e

mkdir -p "$build_dir"
cd ./$build_dir || return 1

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
