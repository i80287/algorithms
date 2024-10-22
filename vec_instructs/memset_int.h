#if !defined(MEMSET_INT_H)
#define MEMSET_INT_H 1

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("avx")
#else
#pragma clang attribute push(__attribute__((target("avx"))), apply_to = function)
#endif  // !__clang__
#endif  // __GNUC__

#include <stdint.h>
#include <x86intrin.h>

#include "config_macros.hpp"

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus

#define MEMSET_INT_FUNC_ATTRIBUTES ATTRIBUTE_NOTHROW ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)

MEMSET_INT_FUNC_ATTRIBUTES
void memset_int_avx(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION {
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
    const uint32_t uvalue_32    = (uint32_t)value;
    uintptr_t offset =
        ((uintptr_t)aligned_32_address - (uintptr_t)aligned_4_address) / sizeof(uint32_t);
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

    for (const __m256i value_vector = _mm256_set1_epi32(value); size >= 8; size -= 8) {
        _mm256_store_si256(aligned_32_address, value_vector);
        ++aligned_32_address;
    }

    uint64_t* aligned_8_address = (uint64_t*)aligned_32_address;
    const uint64_t uvalue_64    = ((uint64_t)uvalue_32 << 32) | uvalue_32;
    switch (size / 2) {
        case 3:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
#if defined(__GNUC__)
            __attribute__((fallthrough));
#endif
        case 2:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
#if defined(__GNUC__)
            __attribute__((fallthrough));
#endif
        case 1:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            break;
        case 0:
            break;
        default:
#if defined(__GNUC__)
            __builtin_unreachable();
#endif
            break;
    }
    if (size % 2) {
        aligned_4_address  = (uint32_t*)aligned_8_address;
        *aligned_4_address = uvalue_32;
    }
}

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif  // !__clang__
#endif  // __GNUC__

MEMSET_INT_FUNC_ATTRIBUTES
void memset_int_default(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION {
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

    while (size >= 4) {
        dst[0] = value;
        dst[1] = value;
        dst[2] = value;
        dst[3] = value;
        dst += 4;
        size -= 4;
    }

    switch (size) {
        case 3:
            dst[2] = value;
#if defined(__GNUC__)
            __attribute__((fallthrough));
#endif
        case 2:
            dst[1] = value;
#if defined(__GNUC__)
            __attribute__((fallthrough));
#endif
        case 1:
            dst[0] = value;
            break;
#if defined(__GNUC__)
        case 0:
            break;
        default:
            __builtin_unreachable();
            break;
#endif
    }
}

#if defined(__GNUC__)

ATTRIBUTE_NODISCARD_WITH_MESSAGE("this function is resolver and should not be used")
ATTRIBUTE_MAYBE_UNUSED
ATTRIBUTE_NOTHROW
ATTRIBUTE_RETURNS_NONNULL
static inline void (*resolve_memset_int(void))(int32_t*, int32_t, size_t) CONFIG_NOEXCEPT_FUNCTION {
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx")) {
        return memset_int_avx;
    }
    return memset_int_default;
}

#if defined(linux) || defined(__linux__)

// clang-format off

MEMSET_INT_FUNC_ATTRIBUTES
#if defined(__cplusplus) && defined(__clang__)
__attribute__((ifunc("resolve_memset_int")))
#else
__attribute__((ifunc("resolve_memset_int")))
#endif
void memset_int(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION;

// clang-format on

#else  // !__linux__

MEMSET_INT_FUNC_ATTRIBUTES
void (*memset_int)(int32_t* dst, int32_t value, size_t size) = NULL;

ATTRIBUTE_NOTHROW
__attribute__((constructor)) static inline void memset_int_initializer(void)
    CONFIG_NOEXCEPT_FUNCTION {
    __builtin_cpu_init();
    memset_int = resolve_memset_int();
}

#endif

#else  // !__GNUC__

MEMSET_INT_FUNC_ATTRIBUTES
ATTRIBUTE_ALWAYS_INLINE
static inline memset_int(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION {
    return memset_int_default(dst, value, size);
}

#endif

#undef MEMSET_INT_FUNC_ATTRIBUTES

#if defined(__cplusplus)
}
#endif  // __cplusplus

#endif  // !MEMSET_INT_H
