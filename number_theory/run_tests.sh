#! /bin/sh

mkdir -p build_tests
cp ./u64-primes.txt ./build_tests/u64-primes.txt
cp ./u128-primes.txt ./build_tests/u128-primes.txt
cd ./build_tests || return 1

# Why we export CC and CXX instead of overwriting CMAKE_C_COMPILER and CMAKE_CXX_COMPILER:
#  https://stackoverflow.com/questions/17275348/how-to-specify-new-gcc-path-for-cmake

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
# GCC_RESULT="$(cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . && make all --jobs 4 && make test)"
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . && make all --jobs 4 && make test

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
# CLANG_RESULT="$(cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . && make all --jobs 4 && make test)"
cmake -DCMAKE_BUILD_TYPE=Release -S .. -B . && make all --jobs 4 && make test

# return "$((GCC_RESULT | CLANG_RESULT))"
