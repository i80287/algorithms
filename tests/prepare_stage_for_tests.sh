#!/usr/bin/env bash

function prepare_tests_data_and_cd_to_build_dir() {
    build_dir="$1"
    if [ -d "$build_dir" ]; then rm -r "$build_dir"; fi
    mkdir -p "$build_dir"
    cp ../number_theory/u64-primes.txt "./$build_dir/u64-primes.txt"
    cp ../number_theory/u128-primes.txt "./$build_dir/u128-primes.txt"
    cp ../tf_idf_actrie/Anglo_Saxon_Chronicle.txt "./$build_dir/Anglo_Saxon_Chronicle.txt"
    cp .clang-tidy "./$build_dir"
    pushd "./$build_dir" || exit 1
}

function leave_test() {
    popd || exit 1
}
