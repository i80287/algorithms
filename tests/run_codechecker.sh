#!/usr/bin/env bash

set -e

build_dir=cmake-build-codechecker
rm -rf $build_dir
mkdir -p $build_dir
cp ./.clang-tidy ./$build_dir
cd ./$build_dir || exit 1

OLDIFS=$IFS
IFS=','
for cc_and_cxx in \
    clang,clang++ \
    gcc,g++ \
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
        -D UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS=ON \
        -D CMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -S .. \
        -B "$cmake_build_dir"
    exported_compile_commands="./$cmake_build_dir/compile_commands.json"
    if [ ! -e "$exported_compile_commands" ]; then
        echo "Config with compilation commands was not created by the cmake at $exported_compile_commands"
        exit 1
    fi
    report_file="./report_$c_compiler"
    parsed_report_file="./parsed_report_$c_compiler"
    parsed_html_report_dir="./parsed_html_reports_$c_compiler"
    set +e
    CodeChecker analyze \
        "$exported_compile_commands" \
        --analyzer-config "cppcheck:addons=$(pwd)/../cppcheck/addons/misc.py" \
        --analyzer-config "cppcheck:addons=$(pwd)/../cppcheck/addons/y2038.py" \
        --analyzer-config "cppcheck:addons=$(pwd)/../cppcheck/addons/misra.py" \
        --analyzer-config clang-tidy:take-config-from-directory=true \
        -o "$report_file"

    echo "Done analyzing (compiler = $c_compiler), starting to parse results"
    CodeChecker parse --print-steps "$report_file" >"$parsed_report_file"
    CodeChecker parse --export html --output "$parsed_html_report_dir" "$report_file"
    set -e
done
IFS=$OLDIFS
