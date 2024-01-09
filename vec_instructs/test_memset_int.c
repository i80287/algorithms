#include "memset_int.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
#if defined(__GNUC__)
    assert(__builtin_cpu_supports("avx"));
#endif
    int32_t buffer[513];
    const size_t arr_len = 512;
    const int32_t k = atoi("-1345452112");

    for (size_t offset = 0; offset <= 1; offset++) {
        // offset = 0 => arr is aligned on a 8-byte boundary
        // offset = 1 => arr is aligned on a 4-byte boundary
        int32_t* arr = &buffer[offset];
        for (size_t i = 0; i <= arr_len; i++) {
            memset(arr, 0, arr_len * sizeof(int32_t));
            memset_int(arr, k, i);
            for (size_t j = 0; j < i; j++) {
                assert(arr[j] == k);
            }
            for (size_t j = i; j < arr_len; j++) {
                assert(arr[j] == 0);
            }
        }
    }
}

// clang -g3 .\test_memset_int.c -O2 -fsanitize="address,undefined" -Wall -Wextra -Wpedantic -Werror -Wunused -pedantic-errors -Wconversion -Wshadow -Wnull-dereference -Wundef -Wwrite-strings -Wbad-function-cast -Wsign-conversion -Wmissing-noreturn -Wunreachable-code -Wint-conversion -Warray-bounds -o test_memset_int.exe
