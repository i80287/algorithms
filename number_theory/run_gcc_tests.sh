#! /bin/sh

if [ -d cmake-build-tests-gcc ]; then rm -r cmake-build-tests-gcc; fi
mkdir -p cmake-build-tests-gcc
cp ./u64-primes.txt ./cmake-build-tests-gcc/u64-primes.txt
cp ./u128-primes.txt ./cmake-build-tests-gcc/u128-primes.txt
cd ./cmake-build-tests-gcc || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
