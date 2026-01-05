#!/usr/bin/env bash

build_dir=cmake-build-codechecker

mkdir -p "$build_dir"
cp ./.clang-tidy ./$build_dir
cd ./$build_dir || exit 1

OLDIFS=$IFS
IFS=','
for cc_and_cxx in \
    clang-18,clang++-18 clang-19,clang++-19 clang-20,clang++-20 \
    gcc-13,g++-13 gcc-14,g++-14 \
    i686-w64-mingw32-gcc-posix,i686-w64-mingw32-g++-posix \
    x86_64-w64-mingw32-gcc-posix,x86_64-w64-mingw32-g++-posix \
; do
    set -- $cc_and_cxx
    c_compiler=$1
    cxx_compiler=$2
    echo "cc = $c_compiler, c++ = $cxx_compiler"

    cmake_build_dir="cmake-build-dir-$c_compiler"
    if [ -d "$cmake_build_dir" ]; then rm -r "$cmake_build_dir"; fi
    mkdir -p "$cmake_build_dir"

    cmake -G Ninja \
        -D CMAKE_BUILD_TYPE=RelWithDebInfo \
        -D CMAKE_C_COMPILER="$1" \
        -D CMAKE_CXX_COMPILER="$2" \
        -D UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS=OFF \
        -D CMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -S .. \
        -B "$cmake_build_dir"
    exported_compile_commands="./$cmake_build_dir/compile_commands.json"
    if [ -e "$exported_compile_commands" ]; then
        cmake --build "$cmake_build_dir" --parallel
        report_file="./report_$c_compiler"
        parsed_report_file="./parsed_report_$c_compiler"
        CodeChecker analyze \
            "$exported_compile_commands" \
            --analyzer-config cppcheck:addons=../cppcheck/addons/misc.py \
            --analyzer-config cppcheck:addons=../cppcheck/addons/y2038.py \
            --analyzer-config clang-tidy:take-config-from-directory=true \
            -o "$report_file"
        CodeChecker parse --print-steps "$report_file" >"$parsed_report_file"
    fi
done
IFS=$OLDIFS
