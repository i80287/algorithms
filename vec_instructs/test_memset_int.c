#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memset_int.h"

int main(void) {
    const size_t arr_len = 512;
    int32_t buffer[arr_len + 1];
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
