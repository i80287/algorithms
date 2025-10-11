#!/usr/bin/env bash

set -e

for force_disable_checks in ON OFF; do
    build_dir=cmake-build-tests-macos-clang
    
    . ./prepare_stage_for_tests.sh
    prepare_tests_data_and_cd_to_build_dir "$build_dir"
    
    cmake -G Ninja \
            -D CMAKE_BUILD_TYPE=RelWithDebInfo \
            -D CMAKE_C_COMPILER=clang \
            -D CMAKE_CXX_COMPILER=clang++ \
            -D UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS=$force_disable_checks \
            -S .. -B . &&
            cmake --build . --parallel &&
            env CTEST_OUTPUT_ON_FAILURE=1 ctest

    leave_test
done
