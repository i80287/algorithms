#!/bin/bash

clang-format -i \
    ./number_theory/*.hpp ./number_theory/*.cpp \
    ./tf_idf_actrie/*.hpp ./tf_idf_actrie/*.cpp \
    -style=file:./.clang-format -fallback-style=Google
