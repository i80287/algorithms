#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

/* Test for gcc >= maj.min, as per __GNUC_PREREQ in glibc */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define CONFIG_GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define CONFIG_GNUC_PREREQ(maj, min) 0
#endif

#if __cplusplus >= 202302L
#define attribute_assume(expr) [[assume(expr)]]
#elif CONFIG_GNUC_PREREQ(13, 0) && !defined(__clang__)
#define attribute_assume(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && defined(__has_builtin) && __has_builtin(__builtin_assume)
// Side effect of expr is discarded
#define attribute_assume(expr) __builtin_assume(expr)
#elif defined(_MSC_VER)
#define attribute_assume(expr) __assume(expr)
#else
#define attribute_assume(expr)
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
#define gcc_attribute_const __attribute__((const))
#else
#define gcc_attribute_const
#endif

#endif  // !CONFIG_MACROS_HPP
