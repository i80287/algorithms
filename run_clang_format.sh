#! /usr/bin/env bash

clang-format -i \
    ./number_theory/*.hpp ./number_theory/*.cpp \
    ./tf_idf_actrie/*.hpp ./tf_idf_actrie/*.cpp \
    ./graphs/HungarianAlgorithm/*.hpp ./graphs/HungarianAlgorithm/*.cpp \
    ./string-switch-map/*.cpp ./string-switch-map/*.hpp ./string-switch-map/tests/*.cpp \
    ./vec_instructs/*.h ./vec_instructs/*.c ./vec_instructs/*.cpp \
    ./misc/*.hpp ./misc/*.cpp \
    ./bstrees/*.cpp ./bstrees/*.cppm \
    -style=file:./.clang-format
