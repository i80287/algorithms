#!/bin/bash

build_dir=cmake-build-tests-gcc-mingw-w64

set -e

if [ -d "$build_dir" ]; then rm -r "$build_dir"; fi
mkdir -p "$build_dir"
cp ../number_theory/u64-primes.txt ./$build_dir/u64-primes.txt
cp ../number_theory/u128-primes.txt ./$build_dir/u128-primes.txt
cd ./$build_dir || return 1

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix -S .. -B . &&
    make all --jobs "$(nproc)"

shopt -s nullglob
for test_executable in *.exe; do
    wine "$test_executable"
done
