#! /usr/bin/env bash

set -e
shopt -s globstar

clang-format -i \
    ./**/*.h ./**/*.c ./**/*.hpp ./**/*.ipp ./**/*.cpp ./**/*.cppm \
    -style=file:./.clang-format
