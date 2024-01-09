#include <assert.h>
#include <stdint.h>

#include "memcount.h"

size_t memcount_slow(const void* src, int c, size_t size) {
    size_t cnt = 0;
    for (const unsigned char* s = (const unsigned char*)src; size != 0;
         ++s, --size) {
        cnt += *s == c;
    }
    return cnt;
}

int main(void) {
#if defined(__GNUC__)
    assert(__builtin_cpu_supports("popcnt"));
    assert(__builtin_cpu_supports("avx"));
    assert(__builtin_cpu_supports("avx2"));
#endif
    const char arr[] =
        "aaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaa"
        "aaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaa"
        "aabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacc"
        "cccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccb"
        "bdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbddddd"
        "ddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaab"
        "aaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaa"
        "aaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaaba"
        "cccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccc"
        "cbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbddd"
        "ddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaa"
        "abaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaa"
        "aaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaa"
        "bacccccccbbdddddddaaabaaaaaaaaaabacccccccbbdddddddaaabaaaaaaaaaabacccc"
        "cccbbdddddddaaabaaaaaaaaaabacccccccbbddddddd";
    const int c = 'a';
    const size_t arr_len = sizeof(arr) - 1;
    for (size_t slice_len = 0; slice_len < arr_len; slice_len++) {
        for (size_t slice_start = 0; slice_start + slice_len <= arr_len; slice_start++) {
            assert(memcount(&arr[slice_start], c, slice_len) == memcount_slow(&arr[slice_start], c, slice_len));
        }
    }
}

// clang -g3 .\test_memcount.c -O2 -fsanitize="address,undefined" -Wall -Wextra -Wpedantic -Werror -Wunused -pedantic-errors -Wconversion -Wshadow -Wnull-dereference -Wundef -Wwrite-strings -Wbad-function-cast -Wsign-conversion -Wmissing-noreturn -Wunreachable-code -Wint-conversion -Warray-bounds -o test_memcount.exe
