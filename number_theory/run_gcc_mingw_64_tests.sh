#! /bin/bash

if [ -d cmake-build-tests-mingw64 ]; then rm -r cmake-build-tests-mingw64; fi
mkdir -p cmake-build-tests-mingw64
cp ./u64-primes.txt ./cmake-build-tests-mingw64/u64-primes.txt
cp ./u128-primes.txt ./cmake-build-tests-mingw64/u128-primes.txt
cd ./cmake-build-tests-mingw64 || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc-posix -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++-posix -S .. -B . &&
    make clean &&
    make all --jobs "$(nproc)"

shopt -s nullglob
for test_executable in *.exe; do
    wine "$test_executable" || exit 1
done
