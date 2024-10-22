#!/bin/bash

build_dir=cmake-build-tests-gcc-mingw-w32

set -e

if [ -d "$build_dir" ]; then rm -r "$build_dir"; fi
mkdir -p "$build_dir"
cp ../number_theory/u64-primes.txt ./$build_dir/u64-primes.txt
cp ../number_theory/u128-primes.txt ./$build_dir/u128-primes.txt
cp ../tf_idf_actrie/Anglo_Saxon_Chronicle.txt ./$build_dir/Anglo_Saxon_Chronicle.txt
cd ./$build_dir

cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=i686-w64-mingw32-gcc-posix -DCMAKE_CXX_COMPILER=i686-w64-mingw32-g++-posix -S .. -B . &&
    make all --jobs "$(nproc)"

shopt -s nullglob
for test_executable in *.exe; do
    echo "Starting $test_executable"
    wine "$test_executable"
done
