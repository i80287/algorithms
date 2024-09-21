#! /bin/sh

if [ -d cmake-build-tests-clang ]; then rm -r cmake-build-tests-clang; fi
mkdir -p cmake-build-tests-clang
cp ./u64-primes.txt ./cmake-build-tests-clang/u64-primes.txt
cp ./u128-primes.txt ./cmake-build-tests-clang/u128-primes.txt
cd ./cmake-build-tests-clang || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -S .. -B . &&
        make all --jobs "$(nproc)" &&
        env CTEST_OUTPUT_ON_FAILURE=1 make test
