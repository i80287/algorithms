#!/bin/bash

build_dir=cmake-build-codechecker
cmake_build_dir=cmake-build-dir

set -e

mkdir -p "$build_dir"
cp ./.clang-tidy ./$build_dir
cd ./$build_dir || exit 1

OLDIFS=$IFS
IFS=','
for cc_and_cxx in clang,clang++ gcc,g++ i686-w64-mingw32-gcc-posix,i686-w64-mingw32-g++-posix x86_64-w64-mingw32-gcc-posix,x86_64-w64-mingw32-g++-posix; do
    if [ -d "$cmake_build_dir" ]; then rm -r "$cmake_build_dir"; fi
    mkdir -p "$cmake_build_dir"
    set -- $cc_and_cxx
    echo "cc = $1, c++ = $2"
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_C_COMPILER="$1" \
        -DCMAKE_CXX_COMPILER="$2" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -S .. \
        -B $cmake_build_dir
    cd ./$cmake_build_dir && make all --jobs "$(nproc)" && cd ..
    CodeChecker analyze \
        ./$cmake_build_dir/compile_commands.json \
        --analyzer-config cppcheck:addons=../cppcheck/addons/misc.py \
        --analyzer-config cppcheck:addons=../cppcheck/addons/y2038.py \
        --analyzer-config clang-tidy:take-config-from-directory=true \
        -o ./report_"$1"
    CodeChecker parse --print-steps ./report_"$1" >./parsed_report_"$1"
done
IFS=$OLDIFS
