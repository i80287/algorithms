#if !defined(MEMSET_INT_H)
#define MEMSET_INT_H 1

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC target("avx")
#else
#pragma clang attribute push (__attribute__((target("avx"))), apply_to=function)
#endif // !__clang__
#endif // __GNUC__

#include <stdint.h>
#include <x86intrin.h>

#if defined(__GNUC__)
__attribute__((nonnull(1)))
#if !defined(__clang__)
__attribute__((access(write_only, 1, 3)))
#endif // !__clang__
#endif // __GNUC__
void memset_int(int32_t* dst, int32_t value, size_t size) {
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#endif
    if (__builtin_expect(dst == NULL, 0))
        return;
#if !defined(__clang__)
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif
#else
    if (dst == NULL)
        return;    
#endif

    uint32_t* aligned_4_address = (uint32_t*)dst;
    __m256i* aligned_32_address = (__m256i*)(((uintptr_t)aligned_4_address + 31) & ~(uintptr_t)31);
    const uint32_t uvalue_32 = (uint32_t)value;
    uintptr_t offset = ((uintptr_t)aligned_32_address - (uintptr_t)aligned_4_address) / 4;
#if defined(__GNUC__)
    if (__builtin_expect(offset > size, 0))
#else
    if (offset > size)
#endif
        offset = size;
    size -= offset;
    while (offset--) {
        *aligned_4_address = uvalue_32;
        ++aligned_4_address;
    }

    const __m256i value_vector = _mm256_set1_epi32(value);
    while (size >= 8) {
        _mm256_store_si256(aligned_32_address, value_vector);
        size -= 8;
        ++aligned_32_address;
    }

    uint64_t* aligned_8_address = (uint64_t*)aligned_32_address;
    const uint64_t uvalue_64 = ((uint64_t)uvalue_32 << 32) | uvalue_32;
    switch (size / 2) {
        case 3:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            __attribute__((fallthrough));
        case 2:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            __attribute__((fallthrough));
        case 1:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            break;
    }
    if (size % 2) {
        aligned_4_address = (uint32_t*)aligned_8_address;
        *aligned_4_address = uvalue_32;
    }
}

#if defined(__GNUC__) && defined(__clang__)
#pragma clang attribute pop
#endif

#if defined(__cplusplus)
}
#endif // __cplusplus

#endif // !MEMSET_INT_H
