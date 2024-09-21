#! /bin/bash

if [ -d build_tests ]; then rm -r build_tests; fi
mkdir -p build_tests
cp ./u64-primes.txt ./build_tests/u64-primes.txt
cp ./u128-primes.txt ./build_tests/u128-primes.txt
cd ./build_tests || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -S .. -B . &&
    make clean &&
    make all --jobs "$(nproc)"
