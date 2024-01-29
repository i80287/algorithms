#if !defined(MEMSET_INT_H)
#define MEMSET_INT_H 1

#if defined(__cplusplus)
extern "C" {
#endif  // __cplusplus

#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("avx")
#else
#pragma clang attribute push(__attribute__((target("avx"))), \
                             apply_to = function)
#endif  // !__clang__
#endif  // __GNUC__

#include <stdint.h>
#include <x86intrin.h>

#if defined(__GNUC__)
#if !defined(__clang__)
#define MEMSET_INT_FUNC_ATTRIBUTES \
    __attribute__((nonnull(1))) __attribute__((access(write_only, 1, 3)))
#else
#define MEMSET_INT_FUNC_ATTRIBUTES __attribute__((nonnull(1)))
#endif  // !__clang__
#else
#define MEMSET_INT_FUNC_ATTRIBUTES
#endif  // __GNUC__

MEMSET_INT_FUNC_ATTRIBUTES
void memset_int_avx(int32_t* dst, int32_t value, size_t size) {
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
    __m256i* aligned_32_address =
        (__m256i*)(((uintptr_t)aligned_4_address + 31) & ~(uintptr_t)31);
    const uint32_t uvalue_32 = (uint32_t)value;
    uintptr_t offset =
        ((uintptr_t)aligned_32_address - (uintptr_t)aligned_4_address) / 4;
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
#if defined(__GNUC__)
        case 0:
            break;
        default:
            __builtin_unreachable();
            break;
#endif
    }
    if (size % 2) {
        aligned_4_address = (uint32_t*)aligned_8_address;
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
void memset_int_default(int32_t* dst, int32_t value, size_t size) {
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

#if 0 && (defined(linux) || defined(__linux__))

__attribute__((unused)) static void (*resolve_memset_int(void))(int32_t*,
                                                                int32_t,
                                                                size_t) {
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx")) {
        return memset_int_avx;
    }
    return memset_int_default;
}

MEMSET_INT_FUNC_ATTRIBUTES
__attribute__((ifunc("resolve_memset_int"))) void memset_int(int32_t* dst,
                                                             int32_t value,
                                                             size_t size);

#else

MEMSET_INT_FUNC_ATTRIBUTES
void (*memset_int)(int32_t* dst, int32_t value, size_t size) = NULL;

__attribute__((constructor)) static inline void memset_int_initializer(void) {
    memset_int =
        __builtin_cpu_supports("avx") ? memset_int_avx : memset_int_default;
}

#endif

#else

MEMSET_INT_FUNC_ATTRIBUTES
static inline memset_int(int32_t* dst, int32_t value, size_t size) {
    return memset_int_default(dst, value, size);
}

#endif

#undef MEMSET_INT_FUNC_ATTRIBUTES

#if defined(__cplusplus)
}
#endif  // __cplusplus

#endif  // !MEMSET_INT_H
