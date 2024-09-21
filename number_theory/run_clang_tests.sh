#! /bin/sh

if [ -d build_tests_clang ]; then rm -r build_tests_clang; fi
mkdir -p build_tests_clang
cp ./u64-primes.txt ./build_tests_clang/u64-primes.txt
cp ./u128-primes.txt ./build_tests_clang/u128-primes.txt
cd ./build_tests_clang || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
