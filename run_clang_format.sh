#! /usr/bin/env bash

clang-format -i \
    ./**/*.h ./**/*.hpp ./**/*.ipp ./**/*.cpp ./**/*.cppm \
    -style=file:./.clang-format
