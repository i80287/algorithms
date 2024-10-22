#ifdef NDEBUG
// cppcheck-suppress [preprocessorErrorDirective]
#error "NDEBUG defined, asserts are off"
#endif

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "memcount.h"

namespace {

size_t memcount_slow(const uint8_t* const src, const uint8_t chr, size_t size) noexcept {
    size_t cnt       = 0;
    const uint32_t c = chr;
    for (const uint8_t* s = src; size > 0; ++s, --size) {
        cnt += *s == c;
    }
    return cnt;
}

}  // namespace

int main() {
    constexpr uint8_t arr[] =
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
    const uint8_t c          = static_cast<uint8_t>(std::atoi("97"));
    constexpr size_t arr_len = sizeof(arr) - 1;
    for (size_t slice_len = 0; slice_len < arr_len; slice_len++) {
        for (size_t slice_start = 0; slice_start + slice_len <= arr_len; slice_start++) {
            assert(memcount(&arr[slice_start], c, slice_len) ==
                   memcount_slow(&arr[slice_start], c, slice_len));
        }
    }
}
