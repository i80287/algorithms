#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

/* Test for gcc >= maj.min, as per __GNUC_PREREQ in glibc */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define CONFIG_GNUC_AT_LEAST(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define CONFIG_GNUC_AT_LEAST(maj, min) 0
#endif

#if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
#define CONFIG_CLANG_AT_LEAST(maj, min) \
    ((__clang_major__ << 16) + __clang_minor__ >= ((maj) << 16) + (min))
#else
#define CONFIG_CLANG_AT_LEAST(maj, min) 0
#endif

/* Test for __has_attribute as per __glibc_has_attribute in glibc */
#if defined(__has_attribute) && (!defined(__clang__) || CONFIG_CLANG_AT_LEAST(2, 5))
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) __has_attribute(attr)
#else
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) 0
#endif

#ifdef __has_cpp_attribute
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) 0
#endif

#if defined(__has_builtin)
#define CONFIG_HAS_BUILTIN(name) __has_builtin(name)
#else
#define CONFIG_HAS_BUILTIN(name) 0
#endif

#if defined(__has_include)
#define CONFIG_HAS_INCLUDE(include_string) __has_include(include_string)
#else
#define CONFIG_HAS_INCLUDE(include_string) 0
#endif

#if CONFIG_HAS_INCLUDE(<version>)
#include <version>
#elif CONFIG_HAS_INCLUDE(<ciso646>)
#include <ciso646>
#elif CONFIG_HAS_INCLUDE(<iso646.h>)
#include <iso646.h>
#endif

// https://en.cppreference.com/w/cpp/feature_test
#if defined(__cplusplus) && __cplusplus >= 201703L
#define CONFIG_HAS_AT_LEAST_CXX_17 1
#else
#define CONFIG_HAS_AT_LEAST_CXX_17 0
#endif

#if defined(__cplusplus) && __cplusplus >= 202002L
#define CONFIG_HAS_AT_LEAST_CXX_20 1
#else
#define CONFIG_HAS_AT_LEAST_CXX_20 0
#endif

#if defined(__cplusplus) && __cplusplus >= 202302L
#define CONFIG_HAS_AT_LEAST_CXX_23 1
#else
#define CONFIG_HAS_AT_LEAST_CXX_23 0
#endif

// https://en.cppreference.com/w/cpp/feature_test
#if defined(__cpp_concepts) && __cpp_concepts >= 202002L
#define CONFIG_HAS_CONCEPTS 1
#else
#define CONFIG_HAS_CONCEPTS 0
#endif

/**
 * Restrict qualifier for the C++ (C has `restrict` keyword since C99)
 */
#if defined(__GNUC__) || defined(__clang__)
#define RESTRICT_QUALIFIER __restrict__
#elif defined(_MSC_VER)
#define RESTRICT_QUALIFIER __restrict
#else
#define RESTRICT_QUALIFIER
#endif

#if defined(__GNUC__) || defined(__clang__)
#define FUNCTION_MACRO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define FUNCTION_MACRO __FUNCSIG__
#else
#define FUNCTION_MACRO __func__
#endif

#if CONFIG_HAS_INCLUDE(<utility>)
#include <utility>
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define CONFIG_UNREACHABLE() std::unreachable()
#elif CONFIG_HAS_AT_LEAST_CXX_23 && CONFIG_HAS_CPP_ATTRIBUTE(assume)
#define CONFIG_UNREACHABLE() [[assume(false)]]
#elif CONFIG_HAS_BUILTIN(__builtin_unreachable)
#define CONFIG_UNREACHABLE() __builtin_unreachable()
#elif CONFIG_HAS_BUILTIN(__builtin_assume)
#define CONFIG_UNREACHABLE() __builtin_assume(false)
#elif CONFIG_GNUC_AT_LEAST(13, 0) && CONFIG_HAS_GCC_ATTRIBUTE(assume)
#define CONFIG_UNREACHABLE() __attribute__((assume(false)))
#elif defined(_MSC_VER)
#define CONFIG_UNREACHABLE() __assume(false)
#else
#define CONFIG_UNREACHABLE() \
    do {                     \
    } while (false)
#endif

#if CONFIG_HAS_AT_LEAST_CXX_23 && CONFIG_HAS_CPP_ATTRIBUTE(assume)
#define ATTRIBUTE_ASSUME(expr) [[assume(expr)]]
#elif CONFIG_GNUC_AT_LEAST(13, 0) && CONFIG_HAS_GCC_ATTRIBUTE(assume)
#define ATTRIBUTE_ASSUME(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && CONFIG_HAS_BUILTIN(__builtin_assume)
// Side effect of expr is discarded
#define ATTRIBUTE_ASSUME(expr) __builtin_assume(expr)
#elif defined(_MSC_VER)
#define ATTRIBUTE_ASSUME(expr) __assume(expr)
#else
#define ATTRIBUTE_ASSUME(expr)    \
    do {                          \
        if (!(expr)) {            \
            CONFIG_UNREACHABLE(); \
        }                         \
    } while (false)
#endif

/* __builtin_expect is in gcc 3.0 */
#if CONFIG_GNUC_AT_LEAST(3, 0) || CONFIG_HAS_BUILTIN(__builtin_expect)

#if defined(likely)
#undef likely
#endif
#if defined(unlikely)
#undef unlikely
#endif

#if defined(__cplusplus)
#define likely(x)   __builtin_expect(static_cast<bool>(x), true)
#define unlikely(x) __builtin_expect(static_cast<bool>(x), false)
#else
#define likely(x)   __builtin_expect((x), true)
#define unlikely(x) __builtin_expect((x), false)
#endif

#else

#if !defined(likely)
#if defined(__cplusplus)
#define likely(x) static_cast<bool>(x)
#else
#define likely(x) !!(x)
#endif
#endif

#if !defined(unlikely)
#if defined(__cplusplus)
#define unlikely(x) static_cast<bool>(x)
#else
#define unlikely(x) !!(x)
#endif
#endif

#endif

/* Copied from the sys/cdefs.h from the glibc */
#if !(defined(__GNUC__) || defined(__clang__))
#define __attribute__(...)
#endif

#if CONFIG_GNUC_AT_LEAST(2, 6) || CONFIG_HAS_GCC_ATTRIBUTE(__const__)
#define ATTRIBUTE_CONST __attribute__((__const__))
#elif (defined(__GNUG__) || defined(__clang__)) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::__const__)
#define ATTRIBUTE_CONST [[gnu::__const__]]
#else
#define ATTRIBUTE_CONST
#endif

#if CONFIG_GNUC_AT_LEAST(2, 7) || CONFIG_HAS_GCC_ATTRIBUTE(unused)
#define ATTRIBUTE_MAYBE_UNUSED __attribute__((unused))
#elif CONFIG_HAS_AT_LEAST_CXX_17
#define ATTRIBUTE_MAYBE_UNUSED [[maybe_unused]]
#else
#define ATTRIBUTE_MAYBE_UNUSED
#endif

#if CONFIG_GNUC_AT_LEAST(2, 96) || CONFIG_HAS_GCC_ATTRIBUTE(pure)
#define ATTRIBUTE_PURE __attribute__((pure))
#elif defined(__GNUG__) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::pure)
#define ATTRIBUTE_PURE [[gnu::pure]]
#else
#define ATTRIBUTE_PURE
#endif

#if CONFIG_GNUC_AT_LEAST(3, 0) || CONFIG_HAS_GCC_ATTRIBUTE(noinline)
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#elif defined(__clang__) && CONFIG_HAS_CPP_ATTRIBUTE(clang::noinline)
#define ATTRIBUTE_NOINLINE [[clang::noinline]]
#elif defined(__GNUG__) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::noinline)
#define ATTRIBUTE_NOINLINE [[gnu::noinline]]
#elif defined(_MSC_VER) && CONFIG_HAS_CPP_ATTRIBUTE(msvc::noinline)
#define ATTRIBUTE_NOINLINE [[msvc::noinline]]
#elif defined(_MSC_VER) && \
    (_MSC_VER >= 1920 || defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 191025017)
#define ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define ATTRIBUTE_NOINLINE
#endif

#if CONFIG_GNUC_AT_LEAST(3, 2) || CONFIG_HAS_GCC_ATTRIBUTE(always_inline)
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(__clang__) && CONFIG_HAS_CPP_ATTRIBUTE(clang::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[clang::always_inline]]
#elif defined(__GNUG__) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#elif defined(_MSC_VER) && CONFIG_HAS_CPP_ATTRIBUTE(msvc::forceinline)
#define ATTRIBUTE_ALWAYS_INLINE [[msvc::forceinline]]
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

#if CONFIG_GNUC_AT_LEAST(4, 3) || CONFIG_HAS_GCC_ATTRIBUTE(cold)
#define ATTRIBUTE_COLD __attribute__((cold))
#elif (defined(__GNUG__) || defined(__clang__)) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::cold)
#define ATTRIBUTE_COLD [[gnu::cold]]
#else
#define ATTRIBUTE_COLD
#endif

#if CONFIG_GNUC_AT_LEAST(4, 4) || CONFIG_HAS_GCC_ATTRIBUTE(hot)
#define ATTRIBUTE_HOT __attribute__((hot))
#elif (defined(__GNUG__) || defined(__clang__)) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::hot)
#define ATTRIBUTE_HOT [[gnu::hot]]
#else
#define ATTRIBUTE_HOT
#endif

/**
 * From glibc:
 * Tell the compiler which arguments to an allocation function
 *  indicate the size of the allocation.
 * Clang docs: https://clang.llvm.org/docs/AttributeReference.html#alloc-size
 */
#if CONFIG_GNUC_AT_LEAST(4, 3) || CONFIG_HAS_GCC_ATTRIBUTE(alloc_size)
#define ATTRIBUTE_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
#elif (defined(__GNUG__) || defined(__clang__)) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::alloc_size)
#define ATTRIBUTE_ALLOC_SIZE(...) [[gnu::alloc_size(__VA_ARGS__)]]
#else
#define ATTRIBUTE_ALLOC_SIZE(...)
#endif

/**
 *  mode in { "read_only", "read_write", "write_only", "none" },
 *  mode "none" is valid iff CONFIG_GNUC_AT_LEAST(11, 0)
 *
 *  memory_argument_pos >= 1
 *  range_size_argument_pos >= 1
 *
 *  See https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html for more info
 */
#if CONFIG_GNUC_AT_LEAST(10, 0)
#define ATTRIBUTE_ACCESS(mode, memory_argument_pos) \
    __attribute__((access(mode, memory_argument_pos)))
#define ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos) \
    __attribute__((access(mode, memory_argument_pos, range_size_argument_pos)))
#else
#define ATTRIBUTE_ACCESS(mode, memory_argument_pos)
#define ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos)
#endif

/**
 *  See https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html
 *   and https://clang.llvm.org/docs/AttributeReference.html#id664 for more info
 */
#if CONFIG_GNUC_AT_LEAST(3, 3) || CONFIG_HAS_GCC_ATTRIBUTE(nonnull)
#define ATTRIBUTE_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#elif CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__GNUG__) || defined(__clang__))
#define ATTRIBUTE_NONNULL(...) [[gnu::nonnull(__VA_ARGS__)]]
#else
#define ATTRIBUTE_NONNULL(...)
#endif

#if CONFIG_GNUC_AT_LEAST(4, 9) || CONFIG_HAS_GCC_ATTRIBUTE(returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#elif (defined(__GNUG__) || defined(__clang__)) && CONFIG_HAS_CPP_ATTRIBUTE(gnu::returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL [[gnu::returns_nonnull]]
#else
#define ATTRIBUTE_RETURNS_NONNULL
#endif

#if defined(__clang__) && CONFIG_HAS_CPP_ATTRIBUTE(clang::lifetimebound)
#define ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#else
#define ATTRIBUTE_LIFETIME_BOUND
#endif

#if defined(__clang__) && CONFIG_HAS_CPP_ATTRIBUTE(clang::reinitializes)
#define ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
#else
#define ATTRIBUTE_REINITIALIZES
#endif

// Copypasted from LLVM's int_endianness.h

/* ===-- int_endianness.h - configuration header for compiler-rt ------------===
 *
 *		       The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file is a configuration header for compiler-rt.
 * This file is not part of the interface of this library.
 *
 * ===----------------------------------------------------------------------===
 */

#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)

/* Clang and GCC provide built-in endianness definitions. */
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 0
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0
#endif /* __BYTE_ORDER__ */

#else /* Compilers other than Clang or GCC. */

#if defined(__SVR4) && defined(__sun)
#include <sys/byteorder.h>

#if defined(_BIG_ENDIAN)
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 0
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    1
#elif defined(_LITTLE_ENDIAN)
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0
#else /* !_LITTLE_ENDIAN */
#error "unknown endianness"
#endif /* !_LITTLE_ENDIAN */

#endif /* Solaris and AuroraUX. */

/* .. */

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__minix)
#include <sys/endian.h>

#if _BYTE_ORDER == _BIG_ENDIAN
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 0
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    1
#elif _BYTE_ORDER == _LITTLE_ENDIAN
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0
#endif /* _BYTE_ORDER */

#endif /* *BSD */

#if defined(__OpenBSD__) || defined(__Bitrig__)
#include <machine/endian.h>

#if _BYTE_ORDER == _BIG_ENDIAN
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 0
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    1
#elif _BYTE_ORDER == _LITTLE_ENDIAN
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0
#endif /* _BYTE_ORDER */

#endif /* OpenBSD and Bitrig. */

/* .. */

/* Mac OSX has __BIG_ENDIAN__ or __LITTLE_ENDIAN__ automatically set by the
 * compiler (at least with GCC) */
#if defined(__APPLE__) || defined(__ellcc__)

#ifdef __BIG_ENDIAN__
#if __BIG_ENDIAN__
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 0
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    1
#endif
#endif /* __BIG_ENDIAN__ */

#ifdef __LITTLE_ENDIAN__
#if __LITTLE_ENDIAN__
#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0
#endif
#endif /* __LITTLE_ENDIAN__ */

#endif /* Mac OSX */

/* .. */

#if defined(_WIN32)

#define CONFIG_BYTE_ORDER_LITTLE_ENDIAN 1
#define CONFIG_BYTE_ORDER_BIG_ENDIAN    0

#endif /* Windows */

#endif /* Clang or GCC. */

/* . */

#if !defined(CONFIG_BYTE_ORDER_LITTLE_ENDIAN) || !defined(CONFIG_BYTE_ORDER_BIG_ENDIAN)
// cppcheck-suppress [preprocessorErrorDirective]
#error Unable to determine endian
#endif /* Check we found an endianness correctly. */

#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
#include <intrin.h>  // for _ReadWriteBarrier
#endif

namespace config {

namespace detail {

#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
ATTRIBUTE_NOINLINE inline void sink_char_ptr(char const volatile*) {}
#endif

}  // namespace detail

template <class T>
ATTRIBUTE_ALWAYS_INLINE inline void do_not_optimize_away(T&& expr) noexcept {
#if (defined(_MSC_VER) || defined(__MINGW32__)) && CONFIG_HAS_INCLUDE(<intrin.h>)
    ::config::detail::sink_char_ptr(&reinterpret_cast<volatile const char&>(expr));
    _ReadWriteBarrier();
#elif defined(__GNUC__)
    asm volatile("" ::"r,m,i"(expr));
#else
    __asm__("" ::"r,m,i"(expr));
#endif
}

}  // namespace config

#if CONFIG_HAS_INCLUDE(<type_traits>)
#include <type_traits>
#endif

namespace config {

ATTRIBUTE_ALWAYS_INLINE constexpr bool is_constant_evaluated() noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L && \
    CONFIG_HAS_INCLUDE(<type_traits>)
    return std::is_constant_evaluated();
#elif CONFIG_HAS_BUILTIN(__builtin_is_constant_evaluated)
    return __builtin_is_constant_evaluated();
#else
    return false;
#endif
}

}  // namespace config

#if CONFIG_HAS_INCLUDE(<utility>)
#include <utility>
#endif

namespace config {

template <class T>
ATTRIBUTE_ALWAYS_INLINE constexpr bool is_gcc_constant_p(ATTRIBUTE_MAYBE_UNUSED T&& expr) noexcept {
#if CONFIG_HAS_BUILTIN(__builtin_constant_p) && CONFIG_HAS_INCLUDE(<utility>)
    return static_cast<bool>(__builtin_constant_p(std::forward<T>(expr)));
#else
    return false;
#endif
}

}  // namespace config

#endif  // !CONFIG_MACROS_HPP
