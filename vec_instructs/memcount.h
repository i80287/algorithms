#if !defined(MEMSET_COUNT_H)
#define MEMSET_COUNT_H 1

#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus
#include <x86intrin.h>
#if defined(__cplusplus)
}
#endif  // __cplusplus

#include "config_macros.hpp"

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus

#define MEMCOUNT_ATTRIBUTES                                                                \
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("return value of the memcount should not be ommited") \
    ATTRIBUTE_NOTHROW ATTRIBUTE_SIZED_ACCESS(read_only, 1, 3) ATTRIBUTE_PURE

#if defined(__GNUC__) || defined(__clang__)
#define FAST_MEMCOUNT_TARGET_ATTRIBUTE __attribute__((target("popcnt,avx,avx2")))
#else
#define FAST_MEMCOUNT_TARGET_ATTRIBUTE
#endif

MEMCOUNT_ATTRIBUTES
FAST_MEMCOUNT_TARGET_ATTRIBUTE
size_t memcount_avx(const uint8_t* const src, const uint8_t chr,
                    size_t size) CONFIG_NOEXCEPT_FUNCTION {
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

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

    const uint8_t* not_aligned_address = src;
    const __m256i* aligned_32_address =
        (const __m256i*)(((uintptr_t)not_aligned_address + 31) & ~(uintptr_t)31);

    uintptr_t mem_offset = (uintptr_t)aligned_32_address - (uintptr_t)not_aligned_address;
#if defined(__GNUC__)
    if (__builtin_expect(mem_offset > size, 0))
#else
    if (mem_offset > size)
#endif
        mem_offset = size;
    size -= mem_offset;

    size_t eq_count = 0;
    for (const uint32_t cmp_chr_u32 = (uint32_t)(unsigned int)(chr); mem_offset--;
         ++not_aligned_address) {
        eq_count += *not_aligned_address == cmp_chr_u32;
    }

    mem_offset   = size % 32;
    size_t steps = size / 32;

    for (const __m256i chr_vector = _mm256_set1_epi8((char)chr); steps > 0;
         --steps, ++aligned_32_address) {
        eq_count += (uint32_t)_popcnt32(
            (uint32_t)_mm256_movemask_epi8(_mm256_cmpeq_epi8(chr_vector, *aligned_32_address)));
    }

    not_aligned_address = (const uint8_t*)aligned_32_address;
    for (const uint32_t cmp_chr_u32 = (uint32_t)(unsigned int)(chr); mem_offset--;
         ++not_aligned_address) {
        eq_count += *not_aligned_address == cmp_chr_u32;
    }

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    return eq_count;
}

#undef FAST_MEMCOUNT_TARGET_ATTRIBUTE

MEMCOUNT_ATTRIBUTES
size_t memcount_default(const uint8_t* const src, const uint8_t chr,
                        size_t size) CONFIG_NOEXCEPT_FUNCTION {
    size_t cnt       = 0;
    const uint32_t c = chr;
    for (const uint8_t* s = src; size > 0; ++s, --size) {
        cnt += *s == c;
    }
    return cnt;
}

#if defined(__GNUC__) || defined(__clang__)

// clang-format off

ATTRIBUTE_NODISCARD_WITH_MESSAGE("this function is resolver and should not be used")
ATTRIBUTE_MAYBE_UNUSED
ATTRIBUTE_NOTHROW
ATTRIBUTE_RETURNS_NONNULL
#if defined(__clang__)
__attribute__((no_sanitize("address", "thread", "memory", "undefined")))
#elif defined(__GNUC__)
__attribute__((no_sanitize_address, no_sanitize_thread, no_sanitize_undefined))
#endif
static inline size_t (*resolve_memcount(void))(const uint8_t*, uint8_t,
                                               size_t) {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("popcnt") ? memcount_avx
                                                                              : memcount_default;
}

#if defined(linux) || defined(__linux__)

MEMCOUNT_ATTRIBUTES
#if defined(__cplusplus) && defined(__clang__)
__attribute__((ifunc("_ZL16resolve_memcountv")))
#else
__attribute__((ifunc("resolve_memcount")))
#endif
size_t memcount(const uint8_t* const src, const uint8_t chr, size_t size) CONFIG_NOEXCEPT_FUNCTION;

#else  // !__linux__

#if defined(__cplusplus)

#if CONFIG_HAS_AT_LEAST_CXX_17
static inline
#endif
size_t (*const memcount)(const uint8_t* const src, const uint8_t chr, size_t size) = resolve_memcount();

#else

size_t (*memcount)(const uint8_t* const src, const uint8_t chr, size_t size) = NULL;

__attribute__((constructor)) static inline void memcount_initializer(void) {
    memcount = resolve_memcount();
}

#endif

#endif

// clang-format on

#else  // !__GNUC__

MEMCOUNT_ATTRIBUTES
ATTRIBUTE_ALWAYS_INLINE
static inline size_t memcount(const uint8_t* const src, const uint8_t chr,
                              size_t size) CONFIG_NOEXCEPT_FUNCTION {
    return memcount_default(src, chr, size);
}

#endif

#if defined(__cplusplus)
}
#endif  // __cplusplus

#endif  // !MEMSET_COUNT_H
