#!/bin/bash

build_dir=cmake-build-codechecker

mkdir -p "$build_dir"
cd ./$build_dir || exit 1

OLDIFS=$IFS
IFS=','
for cc_and_cxx in clang,clang++ gcc,g++ i686-w64-mingw32-gcc-posix,i686-w64-mingw32-g++-posix x86_64-w64-mingw32-gcc-posix,x86_64-w64-mingw32-g++-posix; do
    set -- $cc_and_cxx
    echo "cc = $1, c++ = $2"
    cmake -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER="$1" -DCMAKE_CXX_COMPILER="$2" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S .. -B .
    CodeChecker analyze compile_commands.json -o ./report_"$1"
    CodeChecker parse --print-steps ./report_"$1" >./parsed_report_"$1"
done
IFS=$OLDIFS
