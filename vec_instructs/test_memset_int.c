#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memset_int.h"

enum { kArrLen = 512 };

int main(void) {
    const size_t arr_len = kArrLen;
    int32_t buffer[kArrLen + 1];
    const int32_t k = atoi("-1345452112");

    for (size_t offset = 0; offset <= 1; offset++) {
        // offset = 0 => arr is aligned on a 8-byte boundary
        // offset = 1 => arr is aligned on a 4-byte boundary
        int32_t* const arr = &buffer[offset];
        for (size_t i = 0; i <= arr_len; i++) {
            const uint8_t kMagicByte = 251;
            const int32_t kMagicInt32 = (int32_t)((((uint32_t)kMagicByte) << 24) | (((uint32_t)kMagicByte) << 16) |
                                                  (((uint32_t)kMagicByte) << 8) | (((uint32_t)kMagicByte) << 0));

            memset(arr, kMagicByte, arr_len * sizeof(int32_t));
            memset_int(arr, k, i);
            for (size_t j = 0; j < i; j++) {
                assert(arr[j] == k);
            }
            for (size_t j = i; j < arr_len; j++) {
                assert(arr[j] == kMagicInt32);
            }
        }
    }
}
