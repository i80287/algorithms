#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

#if __cplusplus >= 202302L
#define attribute_assume(expr) [[assume(expr)]]
#elif defined(__GNUC__) && !defined(__clang__)
#define attribute_assume(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && defined(__has_builtin)
#if __has_builtin(__builtin_assume)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wassume"
// Side effect of expr is discarded
#define attribute_assume(expr) __builtin_assume(expr)
#pragma clang diagnostic pop
#else
#define attribute_assume(expr)
#endif
#elif defined(_MSC_VER)
#define attribute_assume(expr) __assume(expr)
#else
#define attribute_assume(expr)
#endif

#if defined(__GNUC__)
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

#if defined(__GNUC__)
#define gcc_attribute_const __attribute__((const))
#else
#define gcc_attribute_const
#endif

#endif  // !CONFIG_MACROS_HPP
