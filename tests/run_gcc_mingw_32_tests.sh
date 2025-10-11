#!/usr/bin/env bash

set -e

for force_disable_checks in ON OFF; do
    build_dir=cmake-build-tests-gcc-mingw-w32
    
    . ./prepare_stage_for_tests.sh
    prepare_tests_data_and_cd_to_build_dir "$build_dir"
    
    cmake -G Ninja \
        -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER=i686-w64-mingw32-gcc-posix \
        -D CMAKE_CXX_COMPILER=i686-w64-mingw32-g++-posix \
        -D UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS=$force_disable_checks \
        -S .. -B . &&
        cmake --build . --parallel
    
    shopt -s nullglob
    for test_executable in *.exe; do
        echo "Starting $test_executable"
        wine "$test_executable"
    done
done
