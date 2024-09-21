#! /bin/sh

if [ -d build_tests_gcc ]; then rm -r build_tests_gcc; fi
mkdir -p build_tests_gcc
cp ./u64-primes.txt ./build_tests_gcc/u64-primes.txt
cp ./u128-primes.txt ./build_tests_gcc/u128-primes.txt
cd ./build_tests_gcc || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
