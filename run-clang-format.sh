#! /bin/sh

clang-format -i ./number_theory/*.hpp ./number_theory/*.cpp -style=file:./.clang-format -fallback-style=Google
