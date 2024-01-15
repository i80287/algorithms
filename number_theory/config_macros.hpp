#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

/* Test for gcc >= maj.min, as per __GNUC_PREREQ in glibc */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define CONFIG_GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define CONFIG_GNUC_PREREQ(maj, min) 0
#endif

/**
 * Restrict qualifier for the C++ (C has `restrict` keyword since C99)
 */
#if defined(__GNUC__)
#define RESTRICT_QUALIFIER __restrict__
#elif defined(__clang__)
#define RESTRICT_QUALIFIER __restrict__
#elif defined(_MSC_VER)
#define RESTRICT_QUALIFIER __restrict
#else
#define RESTRICT_QUALIFIER
#endif

#if defined(__GNUC__)
#define FUNCTION_MACRO __PRETTY_FUNCTION__
#else
#define FUNCTION_MACRO __func__
#endif

#if __cplusplus >= 202302L
#define ATTRIBUTE_ASSUME(expr) [[assume(expr)]]
#elif CONFIG_GNUC_PREREQ(13, 0) && !defined(__clang__)
#define ATTRIBUTE_ASSUME(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && defined(__has_builtin) && __has_builtin(__builtin_assume)
// Side effect of expr is discarded
#define ATTRIBUTE_ASSUME(expr) __builtin_assume(expr)
#elif defined(_MSC_VER)
#define ATTRIBUTE_ASSUME(expr) __assume(expr)
#else
#define ATTRIBUTE_ASSUME(expr)
#endif

/* __builtin_expect is in gcc 3.0 */
#if CONFIG_GNUC_PREREQ(3,0)
#if defined(likely)
#undef likely
#endif
#if defined(unlikely)
#undef unlikely
#endif
#define likely(x)   __builtin_expect(static_cast<bool>(x), true)
#define unlikely(x) __builtin_expect(static_cast<bool>(x), false)
#else
#if !defined(likely)
#define likely(x) static_cast<bool>(x)
#endif
#if !defined(unlikely)
#define unlikely(x) static_cast<bool>(x)
#endif
#endif

#if CONFIG_GNUC_PREREQ(2, 6)
#define GCC_ATTRIBUTE_CONST __attribute__((const))
#else
#define GCC_ATTRIBUTE_CONST
#endif

#endif  // !CONFIG_MACROS_HPP
