#! /bin/sh

mkdir -p build_tests
cp ./u64-primes.txt ./build_tests/u64-primes.txt
cp ./u128-primes.txt ./build_tests/u128-primes.txt
cd ./build_tests || exit
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -S .. -B .
make all --jobs 4
make test
