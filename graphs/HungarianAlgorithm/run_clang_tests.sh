#!/bin/bash

build_dir=cmake-build-tests-clang

set -e

mkdir -p "$build_dir"
cd ./$build_dir || return 1

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
