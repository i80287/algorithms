#! /bin/sh

mkdir -p build_tests
cp ./u64-primes.txt ./build_tests/u64-primes.txt
cp ./u128-primes.txt ./build_tests/u128-primes.txt
cd ./build_tests || return 1

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

cmake -D CMAKE_BUILD_TYPE=Release -S .. -B . && make all --jobs "$(nproc)" && make test
