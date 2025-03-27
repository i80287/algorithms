#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "memset_int.h"

int main() {
    using std::int32_t;
    using std::size_t;
    using std::uint32_t;
    using std::uint8_t;

    constexpr size_t arr_len = 512;
    int32_t buffer[arr_len + 1]{};
    const int32_t k = std::atoi("-1345452112");

    for (size_t offset = 0; offset <= 1; offset++) {
        // offset = 0 => arr is aligned on a 8-byte boundary
        // offset = 1 => arr is aligned on a 4-byte boundary
        int32_t* const arr = &buffer[offset];
        for (size_t i = 0; i <= arr_len; i++) {
            const uint8_t kMagicByte = 251;
            const int32_t kMagicInt32 = static_cast<int32_t>(
                ((uint32_t{kMagicByte}) << 24) | ((uint32_t{kMagicByte}) << 16) |
                ((uint32_t{kMagicByte}) << 8) | ((uint32_t{kMagicByte}) << 0));

            std::memset(arr, kMagicByte, arr_len * sizeof(int32_t));
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
