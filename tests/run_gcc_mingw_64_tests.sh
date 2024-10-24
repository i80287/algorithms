#!/usr/bin/env bash

set -e

build_dir=cmake-build-tests-gcc-mingw-w64

. ./prepare_stage_for_tests.sh
prepare_tests_data_and_cd_to_build_dir "$build_dir"

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo \
    -D CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix \
    -D CMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix \
    -S .. -B . &&
    cmake --build . --parallel "$(nproc)"

shopt -s nullglob
for test_executable in *.exe; do
    echo "Starting $test_executable"
    wine "$test_executable"
done
