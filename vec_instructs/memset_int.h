#if !defined(MEMSET_INT_H)
#define MEMSET_INT_H 1

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

#include "config_macros.hpp"

EXTERN_WITH_C_LINKAGE_BEGIN

#if defined(__cplusplus)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
#endif

#define MEMSET_INT_FUNC_ATTRIBUTES ATTRIBUTE_NOTHROW ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)

#if defined(__GNUC__) || defined(__clang__)
#define FAST_MEMSET_INT_TARGET_ATTRIBUTE __attribute__((target("avx")))
#else
#define FAST_MEMSET_INT_TARGET_ATTRIBUTE
#endif

MEMSET_INT_FUNC_ATTRIBUTES
FAST_MEMSET_INT_TARGET_ATTRIBUTE
static inline void memset_int_avx(int32_t* dst, int32_t value,
                                  size_t size) CONFIG_NOEXCEPT_FUNCTION {
    uint32_t* aligned_4_address = (uint32_t*)dst;
    __m256i* aligned_32_address = (__m256i*)(((uintptr_t)aligned_4_address + 31) & ~(uintptr_t)31);
    const uint32_t uvalue_32    = (uint32_t)value;
    uintptr_t offset =
        ((uintptr_t)aligned_32_address - (uintptr_t)aligned_4_address) / sizeof(uint32_t);
    if (unlikely(offset > size)) {
        offset = size;
    }
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
            ATTRIBUTE_FALLTHROUGH;
        case 2:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            ATTRIBUTE_FALLTHROUGH;
        case 1:
            *aligned_8_address = uvalue_64;
            ++aligned_8_address;
            break;
        case 0:
            break;
        default:
            CONFIG_UNREACHABLE();
            break;
    }
    if (size % 2) {
        aligned_4_address  = (uint32_t*)aligned_8_address;
        *aligned_4_address = uvalue_32;
    }
}

#undef FAST_MEMSET_INT_TARGET_ATTRIBUTE

MEMSET_INT_FUNC_ATTRIBUTES
static inline void memset_int_default(int32_t* dst, int32_t value,
                                      size_t size) CONFIG_NOEXCEPT_FUNCTION {
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
            ATTRIBUTE_FALLTHROUGH;
        case 2:
            dst[1] = value;
            ATTRIBUTE_FALLTHROUGH;
        case 1:
            dst[0] = value;
            break;
        case 0:
            break;
        default:
            CONFIG_UNREACHABLE();
            break;
    }
}

#if defined(__cplusplus)
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)

#if defined(__clang__) && defined(__cplusplus) && !CONFIG_HAS_AT_LEAST_CXX_17
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-compat-mangling"
#endif

ATTRIBUTE_NODISCARD_WITH_MESSAGE("this function is resolver and should not be used")
ATTRIBUTE_MAYBE_UNUSED
ATTRIBUTE_NOTHROW
ATTRIBUTE_RETURNS_NONNULL
#if defined(__clang__)
__attribute__((no_sanitize("address", "thread", "memory", "undefined")))
#elif defined(__GNUC__)
__attribute__((no_sanitize_address, no_sanitize_thread, no_sanitize_undefined))
#endif
static inline void (*resolve_memset_int(void))(int32_t*, int32_t, size_t) CONFIG_NOEXCEPT_FUNCTION {
    __builtin_cpu_init();
    return __builtin_cpu_supports("avx") ? memset_int_avx : memset_int_default;
}

#if defined(__clang__) && defined(__cplusplus) && !CONFIG_HAS_AT_LEAST_CXX_17
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++17-compat-mangling"
#endif

#if defined(linux) || defined(__linux__)

// clang-format off

MEMSET_INT_FUNC_ATTRIBUTES
#if defined(__cplusplus) && defined(__clang__)
__attribute__((ifunc("_ZL18resolve_memset_intv")))
#else
__attribute__((ifunc("resolve_memset_int")))
#endif
static inline void memset_int(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION;

#else  // !__linux__

#if defined(__cplusplus)

#if CONFIG_HAS_AT_LEAST_CXX_17
static inline
#endif
void (*const memset_int)(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION = resolve_memset_int();

#else

void (*memset_int)(int32_t* dst, int32_t value, size_t size) = NULL;

// clang-format on

ATTRIBUTE_NOTHROW
__attribute__((constructor)) static inline void memset_int_initializer(void)
    CONFIG_NOEXCEPT_FUNCTION {
    __builtin_cpu_init();
    memset_int = resolve_memset_int();
}

#endif

#endif

#else  // !__GNUC__

MEMSET_INT_FUNC_ATTRIBUTES
ATTRIBUTE_ALWAYS_INLINE
static inline void memset_int(int32_t* dst, int32_t value, size_t size) CONFIG_NOEXCEPT_FUNCTION {
    memset_int_default(dst, value, size);
}

#endif

#undef MEMSET_INT_FUNC_ATTRIBUTES

EXTERN_WITH_C_LINKAGE_END

#undef EXTERN_WITH_C_LINKAGE_END
#undef EXTERN_WITH_C_LINKAGE_BEGIN

#endif  // !MEMSET_INT_H
