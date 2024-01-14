#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

/**
 * Restrict qualifier for the C++ (C has `restrict` keyword since C99)
 */
#if defined(__GNUC__)
#define restrict_qualifier __restrict__
#elif defined(__clang__)
#define restrict_qualifier __restrict__
#elif defined(_MSC_VER)
#define restrict_qualifier __restrict
#else
#define restrict_qualifier
#endif

#if defined(__GNUC__)
#define function_macro __PRETTY_FUNCTION__
#else
#define function_macro __func__
#endif

#if __cplusplus >= 202302L
#define attribute_assume(expr) [[assume(expr)]]
#elif defined(__GNUC__) && __GNUC__ >= 13 && !defined(__clang__)
#define attribute_assume(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && defined(__has_builtin) && __has_builtin(__builtin_assume)
// Side effect of expr is discarded
#define attribute_assume(expr) __builtin_assume(expr)
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
