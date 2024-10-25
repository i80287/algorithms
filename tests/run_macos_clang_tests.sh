#!/usr/bin/env bash

set -e

build_dir=cmake-build-tests-macos-clang

. ./prepare_stage_for_tests.sh
prepare_tests_data_and_cd_to_build_dir "$build_dir"

cmake -G "Unix Makefiles" \
        -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER=clang \
        -D CMAKE_CXX_COMPILER=clang++ \
        -S .. -B . &&
        cmake --build . --parallel "$(sysctl -n hw.logicalcpu)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
