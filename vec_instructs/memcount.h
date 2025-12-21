#if !defined(MEMCOUNT_H)
#define MEMCOUNT_H 1

#if defined(__cplusplus)
#define EXTERN_WITH_C_LINKAGE_BEGIN extern "C" {
#define EXTERN_WITH_C_LINKAGE_END   }
#else
#define EXTERN_WITH_C_LINKAGE_BEGIN
#define EXTERN_WITH_C_LINKAGE_END
#endif

#include <stdint.h>
EXTERN_WITH_C_LINKAGE_BEGIN
#include <x86intrin.h>
EXTERN_WITH_C_LINKAGE_END

#include "../misc/config_macros.hpp"

EXTERN_WITH_C_LINKAGE_BEGIN

#define MEMCOUNT_ATTRIBUTES                                                                           \
    ATTRIBUTE_NONBLOCKING_FUNCTION ATTRIBUTE_PURE ATTRIBUTE_NOTHROW ATTRIBUTE_NODISCARD_WITH_MESSAGE( \
        "return value of the memcount should not be ommited")                                         \
        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 3)

ATTRIBUTE_TARGET("popcnt,avx,avx2")
MEMCOUNT_ATTRIBUTES
static inline size_t memcount_avx(const uint8_t* const src,
                                  const uint8_t chr,
                                  size_t size) CONFIG_NOEXCEPT_FUNCTION {
#if defined(__cplusplus)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#endif

    const uint8_t* not_aligned_address = src;
    const __m256i* aligned_32_address =
        (const __m256i*)(((uintptr_t)not_aligned_address + 31) & ~(uintptr_t)31);

    uintptr_t mem_offset = (uintptr_t)aligned_32_address - (uintptr_t)not_aligned_address;
    if (unlikely(mem_offset > size)) {
        mem_offset = size;
    }
    size -= mem_offset;

    size_t eq_count = 0;
    for (const uint32_t cmp_chr_u32 = (uint32_t)(unsigned int)(chr); mem_offset--;
         ++not_aligned_address) {
        eq_count += *not_aligned_address == cmp_chr_u32;
    }

    mem_offset = size % 32;
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

#if defined(__cplusplus)
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

    return eq_count;
}

MEMCOUNT_ATTRIBUTES
static inline size_t memcount_default(const uint8_t* const src,
                                      const uint8_t chr,
                                      size_t size) CONFIG_NOEXCEPT_FUNCTION {
    size_t cnt = 0;
    const uint32_t c = chr;
    for (const uint8_t* s = src; size > 0; ++s, --size) {
        cnt += *s == c;
    }
    return cnt;
}

#if defined(__GNUC__) || defined(__clang__)

ATTRIBUTE_RETURNS_NONNULL
ATTRIBUTE_NOTHROW
ATTRIBUTE_NODISCARD_WITH_MESSAGE("this function is resolver and should not be used")
ATTRIBUTE_MAYBE_UNUSED
#if defined(__clang__)
__attribute__((no_sanitize("address", "thread", "memory", "undefined")))
#elif defined(__GNUC__)
__attribute__((no_sanitize_address, no_sanitize_thread, no_sanitize_undefined))
#endif
static inline size_t (*resolve_memcount(void))(const uint8_t*, uint8_t, size_t) {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("popcnt") ? &memcount_avx
                                                                              : &memcount_default;
}

#ifdef __linux__

// clang-format off

MEMCOUNT_ATTRIBUTES
#if defined(__cplusplus) && defined(__clang__)
__attribute__((ifunc("_ZL16resolve_memcountv")))
#else
__attribute__((ifunc("resolve_memcount")))
#endif
static inline size_t memcount(const uint8_t* src, uint8_t chr, size_t size) CONFIG_NOEXCEPT_FUNCTION;

#else  // !__linux__

#if defined(__cplusplus)

#if CONFIG_HAS_AT_LEAST_CXX_17
static inline
#endif
size_t (*const memcount)(const uint8_t* src, uint8_t chr, size_t size) = resolve_memcount();

#else

size_t (*memcount)(const uint8_t* src, uint8_t chr, size_t size) = NULL;

// clang-format on

ATTRIBUTE_NOTHROW
__attribute__((constructor)) static inline void memcount_initializer(void)
    CONFIG_NOEXCEPT_FUNCTION {
    memcount = resolve_memcount();
}

#endif

#endif

#else  // !__GNUC__

MEMCOUNT_ATTRIBUTES
ATTRIBUTE_ALWAYS_INLINE
static inline size_t memcount(const uint8_t* const src,
                              const uint8_t chr,
                              const size_t size) CONFIG_NOEXCEPT_FUNCTION {
    return memcount_default(src, chr, size);
}

#endif

#undef MEMCOUNT_ATTRIBUTES

EXTERN_WITH_C_LINKAGE_END

#undef EXTERN_WITH_C_LINKAGE_END
#undef EXTERN_WITH_C_LINKAGE_BEGIN

#endif  // !MEMCOUNT_H
