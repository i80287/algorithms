#! /bin/sh

mkdir -p build_tests
cd ./build_tests || return 1

cmake -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -S .. -B . \
        && make all --jobs "$(nproc)" \
        && make test
