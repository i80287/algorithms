#!/usr/bin/env bash

build_dir=cmake-build-codechecker
cmake_build_dir=cmake-build-dir

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

    cmake -G Ninja \
        -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER="$1" \
        -D CMAKE_CXX_COMPILER="$2" \
        -D CMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -S .. \
        -B $cmake_build_dir
    if [ -e "./$cmake_build_dir/compile_commands.json" ]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            cpu_count=$(sysctl -n hw.logicalcpu)
        else
            cpu_count=$(nproc)
        fi
        cmake --build ./$cmake_build_dir --parallel "$cpu_count"
        CodeChecker analyze \
            ./$cmake_build_dir/compile_commands.json \
            --analyzer-config cppcheck:addons=../cppcheck/addons/misc.py \
            --analyzer-config cppcheck:addons=../cppcheck/addons/y2038.py \
            --analyzer-config clang-tidy:take-config-from-directory=true \
            -o ./report_"$1"
        CodeChecker parse --print-steps ./report_"$1" >./parsed_report_"$1"
    fi
done
IFS=$OLDIFS
