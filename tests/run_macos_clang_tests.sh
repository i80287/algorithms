#!/bin/bash

build_dir=cmake-build-tests-macos-clang

set -e

if [ -d "$build_dir" ]; then rm -r "$build_dir"; fi
mkdir -p "$build_dir"
cp ../number_theory/u64-primes.txt ./$build_dir/u64-primes.txt
cp ../number_theory/u128-primes.txt ./$build_dir/u128-primes.txt
cp ../tf_idf_actrie/Anglo_Saxon_Chronicle.txt ./$build_dir/Anglo_Saxon_Chronicle.txt
cd ./$build_dir

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S .. -B . &&
        make all --jobs "$(sysctl -n hw.logicalcpu)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
