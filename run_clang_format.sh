#! /usr/bin/env bash

clang-format -i \
    ./**/*.h ./**/*.c ./**/*.hpp ./**/*.ipp ./**/*.cpp ./**/*.cppm \
    -style=file:./.clang-format
