#if !defined(MEMSET_COUNT_H)
#define MEMSET_COUNT_H 1

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("popcnt,avx,avx2")
#else
#pragma clang attribute push(__attribute__((target("popcnt,avx,avx2"))), apply_to = function)
#endif  // !__clang__
#endif  // __GNUC__

#include <stdint.h>
#include <x86intrin.h>

#if defined(__GNUC__)
__attribute__((nonnull(1)))
#if !defined(__clang__)
__attribute__((access(read_only, 1, 3)))
#endif  // !__clang__
#endif  // __GNUC__
size_t memcount(const void* src, int chr, size_t size) {
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull-compare"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#endif
    if (__builtin_expect(src == NULL, 0))
        return 0;
#if !defined(__clang__)
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif
#else
    if (src == NULL)
        return 0;
#endif

    const unsigned char* not_aligned_address = (const unsigned char*)src;
    const __m256i* aligned_32_address = (const __m256i*)(((uintptr_t)not_aligned_address + 31) & ~(uintptr_t)31);

    uintptr_t mem_offset = (uintptr_t)aligned_32_address - (uintptr_t)not_aligned_address;
#if defined(__GNUC__)
    if (__builtin_expect(mem_offset > size, 0))
#else
    if (mem_offset > size)
#endif
        mem_offset = size;
    size -= mem_offset;

    size_t eq_count = 0;
    for (const uint32_t cmp_chr_u32 = (uint32_t)(unsigned int)(chr);
         mem_offset--; ++not_aligned_address) {
        eq_count += *not_aligned_address == cmp_chr_u32;
    }

    mem_offset = size % 32;
    size /= 32;

    for (const __m256i chr_vector = _mm256_set1_epi8((char)chr); size != 0;
         --size, ++aligned_32_address) {
        eq_count += (uint32_t)_popcnt32((uint32_t)_mm256_movemask_epi8(
            _mm256_cmpeq_epi8(chr_vector, *aligned_32_address)));
    }

    not_aligned_address = (const unsigned char*)aligned_32_address;
    for (const uint32_t cmp_chr_u32 = (uint32_t)(unsigned int)(chr);
         mem_offset--; ++not_aligned_address) {
        eq_count += *not_aligned_address == cmp_chr_u32;
    }

    return eq_count;
}

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif  // !__clang__
#endif  // __GNUC__

#if defined(__cplusplus)
}
#endif  // __cplusplus

#endif  // !MEMSET_COUNT_H
