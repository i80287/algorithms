#pragma once

#include "config_macros.hpp"

#ifdef __cplusplus

#if CONFIG_HAS_INCLUDE(<type_traits>)
#include <type_traits>
#endif

#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
#include <intrin.h>  // for _ReadWriteBarrier
#endif

namespace config {

namespace detail {

#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
#if defined(__GNUC__)
// inline function 'sink_char_ptr' given attribute 'noinline'
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
ATTRIBUTE_NOINLINE static inline void sink_char_ptr(char const volatile*) {}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#endif

}  // namespace detail

template <class T>
ATTRIBUTE_ALWAYS_INLINE static inline void do_not_optimize_away(T expr) noexcept {
    // NOLINTBEGIN(hicpp-no-assembler)
#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
    ::config::detail::sink_char_ptr(&reinterpret_cast<volatile const char&>(expr));
    _ReadWriteBarrier();
#elif defined(__GNUC__)
    asm volatile("" ::"r,m,i"(expr));
#else
    __asm__("" ::"r,m,i"(expr));
#endif
    // NOLINTEND(hicpp-no-assembler)
}

}  // namespace config

#else

#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
#include <intrin.h>  // for _ReadWriteBarrier
#if defined(__GNUC__)
// inline function 'sink_char_ptr' given attribute 'noinline'
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
ATTRIBUTE_NOINLINE static inline void sink_char_ptr(
    ATTRIBUTE_MAYBE_UNUSED char const volatile* ptr) {}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#define do_not_optimize_away(expr)                    \
    do {                                              \
        sink_char_ptr((volatile const char*)&(expr)); \
        _ReadWriteBarrier();                          \
    } while (0)

#elif defined(__GNUG__)
#define do_not_optimize_away(expr) asm volatile("" ::"r,m,i"(expr))
#elif defined(__GNUC__)
#define do_not_optimize_away(expr) __asm__ volatile("" ::"r,m,i"(expr))
#else
#define do_not_optimize_away(expr) __asm__("" ::"r,m,i"(expr));
#endif

#endif
