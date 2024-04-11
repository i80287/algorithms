#! /bin/sh

clang-format -i ./number_theory/math_functions.hpp ./number_theory/test_math_functions.cpp -style=file:./.clang-format -fallback-style=Google
