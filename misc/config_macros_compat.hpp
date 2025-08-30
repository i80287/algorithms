#pragma once

#if defined(__GNUC__)
#if defined(__clang__)
#define CONFIG_COMPILER_IS_ANY_CLANG 1
#else
#define CONFIG_COMPILER_IS_GCC 1
#endif
#elif defined(_MSC_VER)
#if defined(__clang__)
#define CONFIG_COMPILER_IS_ANY_CLANG 1
#else
#define CONFIG_COMPILER_IS_MSVC 1
#endif
#endif

#ifndef CONFIG_COMPILER_IS_GCC
#define CONFIG_COMPILER_IS_GCC 0
#endif
#ifndef CONFIG_COMPILER_IS_ANY_CLANG
#define CONFIG_COMPILER_IS_ANY_CLANG 0
#endif
#ifndef CONFIG_COMPILER_IS_MSVC
#define CONFIG_COMPILER_IS_MSVC 0
#endif

#if CONFIG_COMPILER_IS_GCC || CONFIG_COMPILER_IS_ANY_CLANG
#define CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG 1
#else
#define CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG 0
#endif

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

#if defined(_MSC_VER)
#define CONFIG_MSVC_AT_LEAST(maj, min) (_MSC_VER >= ((maj) * 100 + (min)))
#else
#define CONFIG_MSVC_AT_LEAST(maj, min) 0
#endif

/* Test for __has_attribute as per __glibc_has_attribute in glibc */
#if defined(__has_attribute) && (!CONFIG_COMPILER_IS_ANY_CLANG || CONFIG_CLANG_AT_LEAST(4, 5))
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) __has_attribute(attr)
#else
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) 0
#endif

#ifdef __has_builtin
#define CONFIG_HAS_BUILTIN(name) __has_builtin(name)
#else
#define CONFIG_HAS_BUILTIN(name) 0
#endif

#ifdef __has_include
#define CONFIG_HAS_INCLUDE(include_string) __has_include(include_string)
#else
#define CONFIG_HAS_INCLUDE(include_string) 0
#endif

#if CONFIG_HAS_INCLUDE(<version>)
#include <version>
#elif CONFIG_HAS_INCLUDE(<ciso646>)
#include <ciso646>
#endif

#ifdef __has_cpp_attribute
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) 0
#endif

#if CONFIG_HAS_INCLUDE(<utility>)
#include <utility>
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define UNREACHABLE() std::unreachable()
#elif CONFIG_HAS_BUILTIN(__builtin_unreachable)
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE()
#endif

#if CONFIG_HAS_CPP_ATTRIBUTE(assume)
#define CONFIG_ASSUME_STATEMENT(expr) [[assume(expr)]]
#elif CONFIG_COMPILER_IS_ANY_CLANG && CONFIG_HAS_BUILTIN(__builtin_assume)
#define CONFIG_ASSUME_STATEMENT(expr) __builtin_assume(expr)
#elif CONFIG_MSVC_AT_LEAST(19, 20)
#define CONFIG_ASSUME_STATEMENT(expr) __assume(expr)
#else
#define CONFIG_ASSUME_STATEMENT(expr) \
    do {                              \
        if (!(expr)) {                \
            UNREACHABLE();            \
        }                             \
    } while (false)
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::pure)
#define ATTRIBUTE_PURE [[gnu::pure]]
#elif (CONFIG_GNUC_AT_LEAST(2, 96) || CONFIG_CLANG_AT_LEAST(3, 0)) && CONFIG_HAS_GCC_ATTRIBUTE(pure)
#define ATTRIBUTE_PURE __attribute__((pure))
#else
#define ATTRIBUTE_PURE
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::__const__)
#define ATTRIBUTE_CONST [[gnu::__const__]]
#elif (CONFIG_GNUC_AT_LEAST(2, 6) || CONFIG_CLANG_AT_LEAST(3, 2)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(__const__)
#define ATTRIBUTE_CONST __attribute__((__const__))
#else
#define ATTRIBUTE_CONST ATTRIBUTE_PURE
#endif

#if CONFIG_CLANG_AT_LEAST(15, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::noinline)
#define ATTRIBUTE_NOINLINE [[clang::noinline]]
#elif CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::noinline)
#define ATTRIBUTE_NOINLINE [[gnu::noinline]]
#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HAS_CPP_ATTRIBUTE(msvc::noinline)
#define ATTRIBUTE_NOINLINE [[msvc::noinline]]
#elif CONFIG_MSVC_AT_LEAST(19, 20)
#define ATTRIBUTE_NOINLINE __declspec(noinline)
#elif (CONFIG_GNUC_AT_LEAST(3, 0) || CONFIG_CLANG_AT_LEAST(3, 0)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(noinline)
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#else
#define ATTRIBUTE_NOINLINE
#endif

#if CONFIG_CLANG_AT_LEAST(15, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[clang::always_inline]]
#elif CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HAS_CPP_ATTRIBUTE(msvc::forceinline)
#define ATTRIBUTE_ALWAYS_INLINE [[msvc::forceinline]]
#elif (CONFIG_GNUC_AT_LEAST(3, 2) || CONFIG_CLANG_AT_LEAST(3, 0)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(always_inline)
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::cold)
#define ATTRIBUTE_COLD [[gnu::cold]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || CONFIG_CLANG_AT_LEAST(3, 2)) && CONFIG_HAS_GCC_ATTRIBUTE(cold)
#define ATTRIBUTE_COLD __attribute__((cold))
#else
#define ATTRIBUTE_COLD
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::hot)
#define ATTRIBUTE_HOT [[gnu::hot]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || CONFIG_CLANG_AT_LEAST(3, 2)) && CONFIG_HAS_GCC_ATTRIBUTE(hot)
#define ATTRIBUTE_HOT __attribute__((hot))
#else
#define ATTRIBUTE_HOT
#endif

/**
 * From glibc:
 * Tell the compiler which arguments to an allocation function
 *  indicate the size of the allocation.
 * Clang docs: https://clang.llvm.org/docs/AttributeReference.html#alloc-size
 */
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::alloc_size)
#define ATTRIBUTE_ALLOC_SIZE(...) [[gnu::alloc_size(__VA_ARGS__)]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || CONFIG_CLANG_AT_LEAST(4, 0)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(alloc_size)
#define ATTRIBUTE_ALLOC_SIZE(...) __attribute__((alloc_size(__VA_ARGS__)))
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
#if (CONFIG_GNUC_AT_LEAST(10, 0) && defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(access)

#if CONFIG_HAS_CPP_ATTRIBUTE(gnu::access)

#define ATTRIBUTE_ACCESS(mode, memory_argument_pos) [[gnu::access(mode, memory_argument_pos)]]
#define ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos) \
    [[gnu::access(mode, memory_argument_pos, range_size_argument_pos)]]
#else

#define ATTRIBUTE_ACCESS(mode, memory_argument_pos) \
    __attribute__((access(mode, memory_argument_pos)))
#define ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos) \
    __attribute__((access(mode, memory_argument_pos, range_size_argument_pos)))

#endif

#if CONFIG_GNUC_AT_LEAST(11, 0)
#define ATTRIBUTE_ACCESS_NONE(memory_argument_pos) ATTRIBUTE_ACCESS(none, memory_argument_pos)
#else
#define ATTRIBUTE_ACCESS_NONE(memory_argument_pos)
#endif

#else
#define ATTRIBUTE_ACCESS(mode, memory_argument_pos)
#define ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos)
#define ATTRIBUTE_ACCESS_NONE(memory_argument_pos)
#endif

/**
 *  See https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html
 *   and https://clang.llvm.org/docs/AttributeReference.html#id664 for more info
 */
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::nonnull)
#define ATTRIBUTE_NONNULL(...)     [[gnu::nonnull(__VA_ARGS__)]]
#define ATTRIBUTE_NONNULL_ALL_ARGS [[gnu::nonnull]]
#elif (CONFIG_GNUC_AT_LEAST(3, 3) || CONFIG_CLANG_AT_LEAST(3, 0)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(nonnull)
#define ATTRIBUTE_NONNULL(...)     __attribute__((nonnull(__VA_ARGS__)))
#define ATTRIBUTE_NONNULL_ALL_ARGS __attribute__((nonnull))
#else
#define ATTRIBUTE_NONNULL(...)
#define ATTRIBUTE_NONNULL_ALL_ARGS
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL [[gnu::returns_nonnull]]
#elif (CONFIG_GNUC_AT_LEAST(4, 9) || CONFIG_CLANG_AT_LEAST(3, 5)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define ATTRIBUTE_RETURNS_NONNULL
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HAS_CPP_ATTRIBUTE(gnu::target)
#define ATTRIBUTE_TARGET(config_string) [[gnu::target(config_string)]]
#elif (CONFIG_GNUC_AT_LEAST(4, 4) || CONFIG_CLANG_AT_LEAST(3, 7)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(target)
#define ATTRIBUTE_TARGET(config_string) __attribute__((target(config_string)))
#else
#define ATTRIBUTE_TARGET(config_string)
#endif

#if CONFIG_CLANG_AT_LEAST(8, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::lifetimebound)
#define ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HAS_CPP_ATTRIBUTE(msvc::lifetimebound)
#define ATTRIBUTE_LIFETIME_BOUND [[msvc::lifetimebound]]
#else
#define ATTRIBUTE_LIFETIME_BOUND
#endif

#if CONFIG_CLANG_AT_LEAST(20, 1) && CONFIG_HAS_CPP_ATTRIBUTE(clang::lifetime_capture_by)
#define ATTRIBUTE_LIFETIME_CAPTURE_BY(...) [[clang::lifetime_capture_by(__VA_ARGS__)]]
#else
#define ATTRIBUTE_LIFETIME_CAPTURE_BY(...)
#endif

#if CONFIG_CLANG_AT_LEAST(8, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::reinitializes)
#define ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
#else
#define ATTRIBUTE_REINITIALIZES
#endif

#if CONFIG_CLANG_AT_LEAST(10, 0) && CONFIG_HAS_CPP_ATTRIBUTE(gsl::Pointer)
#define ATTRIBUTE_GSL_POINTER(...) [[gsl::Pointer(__VA_ARGS__)]]
#else
#define ATTRIBUTE_GSL_POINTER(...)
#endif

#if CONFIG_CLANG_AT_LEAST(10, 0) && CONFIG_HAS_CPP_ATTRIBUTE(gsl::Owner)
#define ATTRIBUTE_GSL_OWNER(...) [[gsl::Owner(__VA_ARGS__)]]
#else
#define ATTRIBUTE_GSL_OWNER(...)
#endif

#if CONFIG_CLANG_AT_LEAST(17, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::unsafe_buffer_usage)
#define ATTRIBUTE_UNSAFE_BUFFER_USAGE [[clang::unsafe_buffer_usage]]
#else
#define ATTRIBUTE_UNSAFE_BUFFER_USAGE
#endif

#if CONFIG_CLANG_AT_LEAST(19, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::nonblocking)
#define ATTRIBUTE_NONBLOCKING_FUNCTION          [[clang::nonblocking]]
#define ATTRIBUTE_NONBLOCKING_FUNCTION_IF(expr) [[clang::nonblocking(expr)]]
#else
#define ATTRIBUTE_NONBLOCKING_FUNCTION
#define ATTRIBUTE_NONBLOCKING_FUNCTION_IF(expr)
#endif

#if CONFIG_CLANG_AT_LEAST(19, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::nonallocating)
#define ATTRIBUTE_NONALLOCATING_FUNCTION          [[clang::nonallocating]]
#define ATTRIBUTE_NONALLOCATING_FUNCTION_IF(expr) [[clang::nonallocating(expr)]]
#else
#define ATTRIBUTE_NONALLOCATING_FUNCTION
#define ATTRIBUTE_NONALLOCATING_FUNCTION_IF(expr)
#endif

#if CONFIG_CLANG_AT_LEAST(19, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::blocking)
#define ATTRIBUTE_BLOCKING_FUNCTION [[clang::blocking]]
#else
#define ATTRIBUTE_BLOCKING_FUNCTION
#endif

#if CONFIG_CLANG_AT_LEAST(19, 0) && CONFIG_HAS_CPP_ATTRIBUTE(clang::allocating)
#define ATTRIBUTE_ALLOCATING_FUNCTION [[clang::allocating]]
#else
#define ATTRIBUTE_ALLOCATING_FUNCTION
#endif
