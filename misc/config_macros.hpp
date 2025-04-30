#pragma once

#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP

// clang-format off
/**
 *
 * Useful checkers (expand to 0 or 1):
 *
 *   Compiler version checkers:
 *     CONFIG_GNUC_AT_LEAST(major_version, minor_version)
 *     CONFIG_CLANG_AT_LEAST(major_version, minor_version)
 *     CONFIG_MSVC_AT_LEAST(major_version, minor_version)
 *     CONFIG_MSVC_FULL_AT_LEAST(major_version, minor_version, build_number)
 *
 *   Include path checker:
 *     CONFIG_HAS_INCLUDE(include_string)
 *
 *   Attributes presence checkers:
 *     CONFIG_HAS_C_ATTRIBUTE(attribute_name)
 *     CONFIG_HAS_CPP_ATTRIBUTE(attribute_name)
 *     CONFIG_HAS_GCC_ATTRIBUTE(attribute_name)
 *
 *   Compiler builtin presence checker:
 *     CONFIG_HAS_BUILTIN(builtin_name)
 *
 * CONFIG_COMPILER_ID, which is equal to the one of the following pairwise distinct macros:
 *   CONFIG_CLANG_COMPILER_ID
 *   CONFIG_GCC_COMPILER_ID
 *   CONFIG_CLANG_CL_COMPILER_ID
 *   CONFIG_MSVC_COMPILER_ID
 *   CONFIG_UNKNOWN_COMPILER_ID
 * Each macro CONFIG_*_COMPILER_ID is guaranteed to fit in the [0; 2^15 - 1]
 *
 * Shortcuts based on the CONFIG_COMPILER_ID:
 *   CONFIG_COMPILER_IS_GCC - either 0 or 1
 *   CONFIG_COMPILER_IS_ANY_CLANG - either 0 or 1
 *   CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG - either 0 or 1
 *   CONFIG_COMPILER_IS_MSVC - either 0 or 1
 *
 * Useful flags (expand to 0 or 1):
 *   CONFIG_HAS_AT_LEAST_CXX_11
 *   CONFIG_HAS_AT_LEAST_CXX_14
 *   CONFIG_HAS_AT_LEAST_CXX_17
 *   CONFIG_HAS_AT_LEAST_CXX_20
 *   CONFIG_HAS_AT_LEAST_CXX_23
 *   CONFIG_HAS_AT_LEAST_C_99
 *   CONFIG_HAS_AT_LEAST_C_11
 *   CONFIG_HAS_AT_LEAST_C_17
 *   CONFIG_HAS_AT_LEAST_C_23
 *   CONFIG_COMPILER_SUPPORTS_CONCEPTS
 *   CONFIG_HAS_CONCEPTS = CONFIG_COMPILER_SUPPORTS_CONCEPTS and CONFIG_HAS_INCLUDE(<concepts>)
 *   CONFIG_HAS_EXCEPTIONS
 *   CONFIG_HAS_RTTI
 *
 * Restrict qualifier for C++ (in C expands to `restrict`, in C++ expands to
 *  some implementation-defined qualifier, e.g. __restrict or __restrict__):
 *   RESTRICT_QUALIFIER
 *
 * Current function name macro (like __func__, __PRETTY_FUNCTION__, __FUNGSIG__, etc.):
 *   CONFIG_CURRENT_FUNCTION_NAME
 *
 * Assume statement (use like `CONFIG_ASSUME_STATEMENT(x == 2 && y >= 3);`) functional macro:
 *   CONFIG_ASSUME_STATEMENT(expression)
 *
 * Unreachable functional macro (use like `if (can_not_evaluate_to_true) { CONFIG_UNREACHABLE(); }`)
 *   CONFIG_UNREACHABLE()
 *
 * likely/unlikely functional macros (use like `if (unlikely(ptr == nullptr)) { handle_null_pointer(); }`):
 *   likely(expression)
 *   unlikely(expression)
 *
 * Attributes:
 *   ATTRIBUTE_CONST
 *   ATTRIBUTE_MAYBE_UNUSED
 *   ATTRIBUTE_PURE
 *   ATTRIBUTE_NOINLINE
 *   ATTRIBUTE_ALWAYS_INLINE
 *   ATTRIBUTE_COLD
 *   ATTRIBUTE_HOT
 *   ATTRIBUTE_ALLOC_SIZE(...)
 *   ATTRIBUTE_ACCESS(mode, memory_argument_pos)
 *   ATTRIBUTE_SIZED_ACCESS(mode, memory_argument_pos, range_size_argument_pos)
 *   ATTRIBUTE_ACCESS_NONE(memory_argument_pos)
 *   ATTRIBUTE_NONNULL(...)
 *   ATTRIBUTE_NONNULL_ALL_ARGS
 *   ATTRIBUTE_RETURNS_NONNULL
 *   ATTRIBUTE_TARGET(config_string)
 *   ATTRIBUTE_LIFETIME_BOUND
 *   ATTRIBUTE_LIFETIME_CAPTURE_BY(...)
 *   ATTRIBUTE_REINITIALIZES
 *   ATTRIBUTE_GSL_POINTER
 *   ATTRIBUTE_GSL_OWNER
 *   ATTRIBUTE_CORO_AWAIT_ELIDABLE
 *   ATTRIBUTE_CORO_AWAIT_ELIDABLE_ARGUMENT
 *   ATTRIBUTE_NO_SPECIALIZATIONS_ALLOWED
 *   ATTRIBUTE_NODISCARD
 *   ATTRIBUTE_NODISCARD_WITH_MESSAGE(message)
 *   ATTRIBUTE_NORETURN
 *   ATTRIBUTE_FALLTHROUGH
 *   ATTRIBUTE_NOTHROW
 *
 * Noexcept specifier (useful if noexcept function is intended to be used in both C/C++):
 *   CONFIG_NOEXCEPT_FUNCTION
 *
 * Clang nullability qualifiers extension:
 *   CONFIG_CLANG_NONNULL_QUALIFIER
 *   CONFIG_CLANG_NULLABLE_QUALIFIER
 *
 * Platform byte order macros (expand to 0 or 1):
 *   CONFIG_BYTE_ORDER_LITTLE_ENDIAN
 *   CONFIG_BYTE_ORDER_BIG_ENDIAN
 *
 * Functions for the C++:
 *   constexpr bool config::is_constant_evaluated() noexcept
 *   constexpr bool config::is_gcc_constant_p<trivial_type>(trivial_type value) noexcept
 *
 **/
// clang-format on

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CONFIG_CLANG_COMPILER_ID    4390
#define CONFIG_GCC_COMPILER_ID      9428
#define CONFIG_CLANG_CL_COMPILER_ID 3493
#define CONFIG_MSVC_COMPILER_ID     1073
#define CONFIG_UNKNOWN_COMPILER_ID  7490

#if defined(__GNUC__)
#if defined(__clang__)
#define CONFIG_COMPILER_ID CONFIG_CLANG_COMPILER_ID
#else
#define CONFIG_COMPILER_ID CONFIG_GCC_COMPILER_ID
#endif
#elif defined(_MSC_VER)
#if defined(__clang__)
#define CONFIG_COMPILER_ID CONFIG_CLANG_CL_COMPILER_ID
#else
#define CONFIG_COMPILER_ID CONFIG_MSVC_COMPILER_ID
#endif
#else
#define CONFIG_COMPILER_ID CONFIG_UNKNOWN_COMPILER_ID
#endif

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#define CONFIG_COMPILER_IS_GCC       1
#define CONFIG_COMPILER_IS_ANY_CLANG 0
#define CONFIG_COMPILER_IS_MSVC      0
#elif CONFIG_COMPILER_ID == CONFIG_CLANG_COMPILER_ID || \
    CONFIG_COMPILER_ID == CONFIG_CLANG_CL_COMPILER_ID
#define CONFIG_COMPILER_IS_GCC       0
#define CONFIG_COMPILER_IS_ANY_CLANG 1
#define CONFIG_COMPILER_IS_MSVC      0
#elif CONFIG_COMPILER_ID == CONFIG_MSVC_COMPILER_ID
#define CONFIG_COMPILER_IS_GCC       0
#define CONFIG_COMPILER_IS_ANY_CLANG 0
#define CONFIG_COMPILER_IS_MSVC      1
#else
#define CONFIG_COMPILER_IS_GCC       0
#define CONFIG_COMPILER_IS_ANY_CLANG 0
#define CONFIG_COMPILER_IS_MSVC      0
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

#if defined(_MSC_FULL_VER)
#define CONFIG_MSVC_FULL_AT_LEAST(maj, min, build_number) \
    (_MSC_FULL_VER >= ((maj) * 10000000 + (min) * 100000 + (build_number)))
#else
#define CONFIG_MSVC_FULL_AT_LEAST(maj, min, build_number) 0
#endif

/* Test for __has_attribute as per __glibc_has_attribute in glibc */
#if defined(__has_attribute) && (!defined(__clang__) || CONFIG_CLANG_AT_LEAST(4, 5))
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) __has_attribute(attr)
#else
#define CONFIG_HAS_GCC_ATTRIBUTE(attr) 0
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

// https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros
// https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170

#if defined(__cplusplus)

#if CONFIG_COMPILER_IS_MSVC && defined(_MSVC_LANG)

#if _MSVC_LANG >= 201103L
#define CONFIG_HAS_AT_LEAST_CXX_11 1
#endif

#if _MSVC_LANG >= 201402L
#define CONFIG_HAS_AT_LEAST_CXX_14 1
#endif

#if _MSVC_LANG >= 201703L
#define CONFIG_HAS_AT_LEAST_CXX_17 1
#endif

#if _MSVC_LANG >= 202002L
#define CONFIG_HAS_AT_LEAST_CXX_20 1
#endif

#if _MSVC_LANG >= 202302L
#define CONFIG_HAS_AT_LEAST_CXX_23 1
#endif

#else

#if __cplusplus >= 201103L
#define CONFIG_HAS_AT_LEAST_CXX_11 1
#endif

#if __cplusplus >= 201402L
#define CONFIG_HAS_AT_LEAST_CXX_14 1
#endif

#if __cplusplus >= 201703L
#define CONFIG_HAS_AT_LEAST_CXX_17 1
#endif

#if __cplusplus >= 202002L
#define CONFIG_HAS_AT_LEAST_CXX_20 1
#endif

#if __cplusplus >= 202302L
#define CONFIG_HAS_AT_LEAST_CXX_23 1
#endif

#endif

#endif

#ifndef CONFIG_HAS_AT_LEAST_CXX_11
#define CONFIG_HAS_AT_LEAST_CXX_11 0
#endif
#ifndef CONFIG_HAS_AT_LEAST_CXX_14
#define CONFIG_HAS_AT_LEAST_CXX_14 0
#endif
#ifndef CONFIG_HAS_AT_LEAST_CXX_17
#define CONFIG_HAS_AT_LEAST_CXX_17 0
#endif
#ifndef CONFIG_HAS_AT_LEAST_CXX_20
#define CONFIG_HAS_AT_LEAST_CXX_20 0
#endif
#ifndef CONFIG_HAS_AT_LEAST_CXX_23
#define CONFIG_HAS_AT_LEAST_CXX_23 0
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define CONFIG_HAS_AT_LEAST_C_99 1
#else
#define CONFIG_HAS_AT_LEAST_C_99 0
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define CONFIG_HAS_AT_LEAST_C_11 1
#else
#define CONFIG_HAS_AT_LEAST_C_11 0
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define CONFIG_HAS_AT_LEAST_C_17 1
#else
#define CONFIG_HAS_AT_LEAST_C_17 0
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
#define CONFIG_HAS_AT_LEAST_C_23 1
#else
#define CONFIG_HAS_AT_LEAST_C_23 0
#endif

#if CONFIG_HAS_AT_LEAST_C_23 && defined(__has_c_attribute)
#define CONFIG_HAS_C_ATTRIBUTE(attr) __has_c_attribute(attr)
#else
#define CONFIG_HAS_C_ATTRIBUTE(attr) 0
#endif

#if CONFIG_HAS_AT_LEAST_CXX_11 && defined(__has_cpp_attribute)
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) __has_cpp_attribute(attr)
#else
#define CONFIG_HAS_CPP_ATTRIBUTE(attr) 0
#endif

#define CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_VERSIONS(attr, cxx_min_version_satisfied, \
                                                                c_min_version_satisfied)         \
    ((cxx_min_version_satisfied && CONFIG_HAS_CPP_ATTRIBUTE(attr)) ||                            \
     (c_min_version_satisfied && CONFIG_HAS_C_ATTRIBUTE(attr)))

#define CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_CXX_VERSION(attr,                      \
                                                                   cxx_min_version_satisfied) \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_VERSIONS(attr, cxx_min_version_satisfied,  \
                                                            CONFIG_HAS_AT_LEAST_C_23)

#define CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(attr) \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_CXX_VERSION(attr, CONFIG_HAS_AT_LEAST_CXX_11)

// https://en.cppreference.com/w/cpp/feature_test
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_concepts) && __cpp_concepts >= 201907L
#define CONFIG_COMPILER_SUPPORTS_CONCEPTS 1
#else
#define CONFIG_COMPILER_SUPPORTS_CONCEPTS 0
#endif

#if CONFIG_COMPILER_SUPPORTS_CONCEPTS && CONFIG_HAS_INCLUDE(<concepts>)
#define CONFIG_HAS_CONCEPTS 1
#else
#define CONFIG_HAS_CONCEPTS 0
#endif

#if defined(__cplusplus) && defined(__cpp_exceptions) && __cpp_exceptions == 199711L
#define CONFIG_HAS_EXCEPTIONS 1
#else
#define CONFIG_HAS_EXCEPTIONS 0
#endif

#if defined(__cplusplus) && defined(__cpp_rtti) && __cpp_rtti == 199711L
#define CONFIG_HAS_RTTI 1
#else
#define CONFIG_HAS_RTTI 0
#endif

#if CONFIG_HAS_AT_LEAST_C_99
#define RESTRICT_QUALIFIER restrict
#elif CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#define RESTRICT_QUALIFIER __restrict__
#elif CONFIG_COMPILER_IS_MSVC
#define RESTRICT_QUALIFIER __restrict
#else
#define RESTRICT_QUALIFIER
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#define CONFIG_CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#elif CONFIG_COMPILER_IS_MSVC
#define CONFIG_CURRENT_FUNCTION_NAME __FUNCSIG__
#else
#define CONFIG_CURRENT_FUNCTION_NAME __func__
#endif

#if CONFIG_HAS_INCLUDE(<utility>)
#include <utility>
#endif

#if CONFIG_HAS_AT_LEAST_CXX_23 && CONFIG_HAS_CPP_ATTRIBUTE(assume)
#define CONFIG_ASSUME_STATEMENT(expr) [[assume(expr)]]
#elif defined(__clang__) && CONFIG_HAS_BUILTIN(__builtin_assume)
#define CONFIG_ASSUME_STATEMENT(expr) __builtin_assume(expr)
#elif CONFIG_GNUC_AT_LEAST(13, 0) && CONFIG_HAS_GCC_ATTRIBUTE(assume)
#if CONFIG_HAS_AT_LEAST_CXX_11 || CONFIG_HAS_AT_LEAST_C_23
#define CONFIG_ASSUME_STATEMENT(expr)       \
    do {                                    \
        if (!(expr)) {                      \
            __attribute__((assume(false))); \
        }                                   \
    } while (false)
#else
#define CONFIG_ASSUME_STATEMENT(expr)   \
    do {                                \
        if (!(expr)) {                  \
            __attribute__((assume(0))); \
        }                               \
    } while (0)
#endif
#elif CONFIG_COMPILER_IS_MSVC
#define CONFIG_ASSUME_STATEMENT(expr) __assume(expr)
#else
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define CONFIG_ASSUME_STATEMENT(expr) \
    do {                              \
        if (!(expr)) {                \
            std::unreachable();       \
        }                             \
    } while (false)
#elif CONFIG_HAS_BUILTIN(__builtin_unreachable)
#if CONFIG_HAS_AT_LEAST_CXX_11 || CONFIG_HAS_AT_LEAST_C_23
#define CONFIG_ASSUME_STATEMENT(expr) \
    do {                              \
        if (!(expr)) {                \
            __builtin_unreachable();  \
        }                             \
    } while (false)
#else
#define CONFIG_ASSUME_STATEMENT(expr) \
    do {                              \
        if (!(expr)) {                \
            __builtin_unreachable();  \
        }                             \
    } while (0)
#endif
#else
#define CONFIG_ASSUME_STATEMENT(expr)
#endif
#endif

#if CONFIG_HAS_AT_LEAST_CXX_11 || CONFIG_HAS_AT_LEAST_C_23
#define CONFIG_UNREACHABLE() CONFIG_ASSUME_STATEMENT(false)
#else
#define CONFIG_UNREACHABLE() CONFIG_ASSUME_STATEMENT(0)
#endif

/* __builtin_expect is in gcc 3.0 */
#if CONFIG_GNUC_AT_LEAST(3, 0) || CONFIG_HAS_BUILTIN(__builtin_expect)

#if defined(likely)
#undef likely
#endif
#if defined(unlikely)
#undef unlikely
#endif

#if CONFIG_HAS_AT_LEAST_CXX_11
#if CONFIG_GNUC_AT_LEAST(12, 1) || defined(__clang__)
#define likely(x)   __builtin_expect(bool{(x)}, true)
#define unlikely(x) __builtin_expect(bool{(x)}, false)
#else
#define likely(x)   __builtin_expect(static_cast<bool>(x), true)
#define unlikely(x) __builtin_expect(static_cast<bool>(x), false)
#endif
#else
#define likely(x)   __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#endif

#elif !defined(likely) || !defined(likely)

#if defined(likely)
#undef likely
#endif
#if defined(unlikely)
#undef unlikely
#endif

// clang-format off

#if CONFIG_HAS_AT_LEAST_CXX_11
#if CONFIG_GNUC_AT_LEAST(12, 1) || defined(__clang__)
#define likely(x)   bool{(x)}
#define unlikely(x) bool{(x)}
#else
#define likely(x)   static_cast<bool>(x)
#define unlikely(x) static_cast<bool>(x)
#endif
#else
#define likely(x)   !!(x)
#define unlikely(x) !!(x)
#endif

// clang-format on

#endif

/* Copied from the sys/cdefs.h from the glibc */
#if !CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#define __attribute__(...)
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::__const__)
#define ATTRIBUTE_CONST [[gnu::__const__]]
#elif (CONFIG_GNUC_AT_LEAST(2, 6) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(__const__)
#define ATTRIBUTE_CONST __attribute__((__const__))
#else
#define ATTRIBUTE_CONST
#endif

#if CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_CXX_VERSION(maybe_unused, \
                                                               CONFIG_HAS_AT_LEAST_CXX_17)
#define ATTRIBUTE_MAYBE_UNUSED [[maybe_unused]]
#elif (CONFIG_GNUC_AT_LEAST(2, 7) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(unused)
#define ATTRIBUTE_MAYBE_UNUSED __attribute__((unused))
#else
#define ATTRIBUTE_MAYBE_UNUSED
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::pure)
#define ATTRIBUTE_PURE [[gnu::pure]]
#elif (CONFIG_GNUC_AT_LEAST(2, 96) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(pure)
#define ATTRIBUTE_PURE __attribute__((pure))
#else
#define ATTRIBUTE_PURE
#endif

#if CONFIG_COMPILER_IS_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::noinline)
#define ATTRIBUTE_NOINLINE [[clang::noinline]]
#elif CONFIG_COMPILER_IS_GCC && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::noinline)
#define ATTRIBUTE_NOINLINE [[gnu::noinline]]
#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(msvc::noinline)
#define ATTRIBUTE_NOINLINE [[msvc::noinline]]
#elif CONFIG_MSVC_AT_LEAST(19, 20) && CONFIG_MSVC_FULL_AT_LEAST(19, 10, 25017)
#define ATTRIBUTE_NOINLINE __declspec(noinline)
#elif (CONFIG_GNUC_AT_LEAST(3, 0) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(noinline)
#define ATTRIBUTE_NOINLINE __attribute__((noinline))
#else
#define ATTRIBUTE_NOINLINE
#endif

#if CONFIG_COMPILER_IS_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[clang::always_inline]]
#elif CONFIG_COMPILER_IS_GCC && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::always_inline)
#define ATTRIBUTE_ALWAYS_INLINE [[gnu::always_inline]]
#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(msvc::forceinline)
#define ATTRIBUTE_ALWAYS_INLINE [[msvc::forceinline]]
#elif (CONFIG_GNUC_AT_LEAST(3, 2) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(always_inline)
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::cold)
#define ATTRIBUTE_COLD [[gnu::cold]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(cold)
#define ATTRIBUTE_COLD __attribute__((cold))
#else
#define ATTRIBUTE_COLD
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::hot)
#define ATTRIBUTE_HOT [[gnu::hot]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(hot)
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
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::alloc_size)
#define ATTRIBUTE_ALLOC_SIZE(...) [[gnu::alloc_size(__VA_ARGS__)]]
#elif (CONFIG_GNUC_AT_LEAST(4, 3) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(alloc_size)
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

#if CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::access)

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
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::nonnull)
#define ATTRIBUTE_NONNULL(...)     [[gnu::nonnull(__VA_ARGS__)]]
#define ATTRIBUTE_NONNULL_ALL_ARGS [[gnu::nonnull]]
#elif (CONFIG_GNUC_AT_LEAST(3, 3) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(nonnull)
#define ATTRIBUTE_NONNULL(...)     __attribute__((nonnull(__VA_ARGS__)))
#define ATTRIBUTE_NONNULL_ALL_ARGS __attribute__((nonnull))
#else
#define ATTRIBUTE_NONNULL(...)
#define ATTRIBUTE_NONNULL_ALL_ARGS
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL [[gnu::returns_nonnull]]
#elif (CONFIG_GNUC_AT_LEAST(4, 9) || defined(__clang__)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(returns_nonnull)
#define ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define ATTRIBUTE_RETURNS_NONNULL
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::target)
#define ATTRIBUTE_TARGET(config_string) [[gnu::target(config_string)]]
#elif (CONFIG_GNUC_AT_LEAST(4, 4) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(target)
#define ATTRIBUTE_TARGET(config_string) __attribute__((target(config_string)))
#else
#define ATTRIBUTE_TARGET(config_string)
#endif

#if defined(__clang__) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::lifetimebound)
#define ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#elif defined(_MSC_VER) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(msvc::lifetimebound)
#define ATTRIBUTE_LIFETIME_BOUND [[msvc::lifetimebound]]
#else
#define ATTRIBUTE_LIFETIME_BOUND
#endif

#if CONFIG_CLANG_AT_LEAST(20, 1) && \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::lifetime_capture_by)
#define ATTRIBUTE_LIFETIME_CAPTURE_BY(...) [[clang::lifetime_capture_by(__VA_ARGS__)]]
#else
#define ATTRIBUTE_LIFETIME_CAPTURE_BY(...)
#endif

#if defined(__clang__) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::reinitializes)
#define ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
#else
#define ATTRIBUTE_REINITIALIZES
#endif

#if CONFIG_CLANG_AT_LEAST(10, 0) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gsl::Pointer)
#define ATTRIBUTE_GSL_POINTER [[gsl::Pointer]]
#else
#define ATTRIBUTE_GSL_POINTER
#endif

#if CONFIG_CLANG_AT_LEAST(10, 0) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gsl::Owner)
#define ATTRIBUTE_GSL_OWNER [[gsl::Owner]]
#else
#define ATTRIBUTE_GSL_OWNER
#endif

#if CONFIG_CLANG_AT_LEAST(20, 1) && \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::coro_await_elidable)
#define ATTRIBUTE_CORO_AWAIT_ELIDABLE [[clang::coro_await_elidable]]
#else
#define ATTRIBUTE_CORO_AWAIT_ELIDABLE
#endif

#if CONFIG_CLANG_AT_LEAST(20, 1) && \
    CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::coro_await_elidable_argument)
#define ATTRIBUTE_CORO_AWAIT_ELIDABLE_ARGUMENT [[clang::coro_await_elidable_argument]]
#else
#define ATTRIBUTE_CORO_AWAIT_ELIDABLE_ARGUMENT
#endif

#if CONFIG_CLANG_AT_LEAST(20, 1) && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(clang::no_specializations)
#define ATTRIBUTE_NO_SPECIALIZATIONS_ALLOWED [[clang::no_specializations]]
#else
#define ATTRIBUTE_NO_SPECIALIZATIONS_ALLOWED
#endif

#if CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_CXX_VERSION(nodiscard, \
                                                               CONFIG_HAS_AT_LEAST_CXX_17)
#define ATTRIBUTE_NODISCARD [[nodiscard]]
#if CONFIG_HAS_AT_LEAST_CXX_20 || CONFIG_HAS_AT_LEAST_C_23
#define ATTRIBUTE_NODISCARD_WITH_MESSAGE(message) [[nodiscard(message)]]
#endif
#elif (CONFIG_GNUC_AT_LEAST(3, 4) || defined(__clang__)) && \
    CONFIG_HAS_GCC_ATTRIBUTE(warn_unused_result)
#define ATTRIBUTE_NODISCARD __attribute__((warn_unused_result))
#else
#define ATTRIBUTE_NODISCARD
#endif

#ifndef ATTRIBUTE_NODISCARD_WITH_MESSAGE
#define ATTRIBUTE_NODISCARD_WITH_MESSAGE(message) ATTRIBUTE_NODISCARD
#endif

#if CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(noreturn)
#define ATTRIBUTE_NORETURN [[noreturn]]
#elif CONFIG_HAS_AT_LEAST_C_11
#define ATTRIBUTE_NORETURN _Noreturn
#elif (CONFIG_GNUC_AT_LEAST(2, 8) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(noreturn)
#define ATTRIBUTE_NORETURN __attribute__((noreturn))
#else
#define ATTRIBUTE_NORETURN
#endif

#if defined(__cplusplus) && (CONFIG_GNUC_AT_LEAST(2, 8) || CONFIG_CLANG_AT_LEAST(4, 0))
#if CONFIG_HAS_AT_LEAST_CXX_11
#define CONFIG_NOEXCEPT_FUNCTION noexcept(true)
#else
#define CONFIG_NOEXCEPT_FUNCTION throw()
#endif
#else
#define CONFIG_NOEXCEPT_FUNCTION
#endif

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE(gnu::nothrow)
#define ATTRIBUTE_NOTHROW [[gnu::nothrow]]
#elif (CONFIG_GNUC_AT_LEAST(3, 4) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(nothrow)
#define ATTRIBUTE_NOTHROW __attribute__((nothrow))
#else
#define ATTRIBUTE_NOTHROW
#endif

#if CONFIG_HELPER_HAS_CPP_STYLE_ATTRIBUTE_WITH_MIN_CXX_VERSION(fallthrough, \
                                                               CONFIG_HAS_AT_LEAST_CXX_17)
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]]
#elif (CONFIG_GNUC_AT_LEAST(7, 1) || defined(__clang__)) && CONFIG_HAS_GCC_ATTRIBUTE(fallthrough)
#define ATTRIBUTE_FALLTHROUGH __attribute__((fallthrough))
#else
#define ATTRIBUTE_FALLTHROUGH
#endif

#if CONFIG_CLANG_AT_LEAST(3, 7)
/*
 * nullability specifier is a Clang extension
 * Possible warning can be suppressed with the following code:
 *
 * #if defined(__clang__)
 * #pragma clang diagnostic push
 * #pragma clang diagnostic ignored "-Wnullability-extension"
 * #endif
 *
 * Some code that uses CONFIG_CLANG_NONNULL_QUALIFIER or CONFIG_CLANG_NULLABLE_QUALIFIER
 *
 * #if defined(__clang__)
 * #pragma clang diagnostic pop
 * #endif
 *
 **/
#define CONFIG_CLANG_NONNULL_QUALIFIER  _Nonnull
#define CONFIG_CLANG_NULLABLE_QUALIFIER _Nullable
#else
#define CONFIG_CLANG_NONNULL_QUALIFIER
#define CONFIG_CLANG_NULLABLE_QUALIFIER
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

#if CONFIG_HAS_AT_LEAST_CXX_11

#if CONFIG_HAS_INCLUDE(<type_traits>)
#include <type_traits>
#endif

namespace config {

ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_NODISCARD constexpr bool is_constant_evaluated() noexcept {
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L && \
    CONFIG_HAS_INCLUDE(<type_traits>)
    return std::is_constant_evaluated();
#elif CONFIG_HAS_BUILTIN(__builtin_is_constant_evaluated)
    return __builtin_is_constant_evaluated();
#else
    return false;
#endif
}

template <class T>
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_NODISCARD constexpr bool is_gcc_constant_p(
    ATTRIBUTE_MAYBE_UNUSED T expr) noexcept {
#if CONFIG_HAS_AT_LEAST_CXX_11 && CONFIG_HAS_INCLUDE(<type_traits>)
    static_assert(
#if CONFIG_HAS_AT_LEAST_CXX_17
        std::is_trivially_copyable_v<T> && std::is_nothrow_copy_constructible_v<T>
#else
        // NOLINTBEGIN(modernize-type-traits)
        std::is_trivial<T>::value
    // NOLINTEND(modernize-type-traits)
#endif
        ,
        "Type passed to the is_gcc_constant_p() should be trivial");
#endif

#if CONFIG_HAS_BUILTIN(__builtin_constant_p)

#if CONFIG_COMPILER_IS_ANY_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wused-but-marked-unused"
#endif

    return static_cast<bool>(__builtin_constant_p(expr));

#if CONFIG_COMPILER_IS_ANY_CLANG
#pragma clang diagnostic pop
#endif

#else
    return false;
#endif
}

}  // namespace config

#endif

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // !CONFIG_MACROS_HPP
