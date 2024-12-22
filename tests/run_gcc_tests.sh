#!/usr/bin/env bash

set -e

build_dir=cmake-build-tests-gcc

. ./prepare_stage_for_tests.sh
prepare_tests_data_and_cd_to_build_dir "$build_dir"

cmake -G Ninja \
        -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER=gcc \
        -D CMAKE_CXX_COMPILER=g++ \
        -S .. -B . &&
        cmake --build . --parallel &&
        env CTEST_OUTPUT_ON_FAILURE=1 ctest
