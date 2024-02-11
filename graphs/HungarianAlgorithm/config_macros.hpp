#ifndef CONFIG_MACROS_HPP
#define CONFIG_MACROS_HPP 1

/* Test for gcc >= maj.min, as per __GNUC_PREREQ in glibc */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define CONFIG_GNUC_PREREQ(maj, min) \
    ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
#define CONFIG_GNUC_PREREQ(maj, min) 0
#endif

/* Test for __has_attribute as per __glibc_has_attribute in glibc */
#if (defined(__has_attribute) &&   \
     (!defined(__clang_major__) || \
      3 < __clang_major__ + (5 <= __clang_minor__)))
#define CONFIG_HAS_ATTRIBUTE(attr) __has_attribute(attr)
#else
#define CONFIG_HAS_ATTRIBUTE(attr) 0
#endif

#if defined(__has_builtin)
#define CONFIG_HAS_BUILTIN(name) __has_builtin(name)
#else
#define CONFIG_HAS_BUILTIN(name) 0
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
#elif defined(_MSC_VER)
#define FUNCTION_MACRO __FUNCSIG__
#else
#define FUNCTION_MACRO __func__
#endif

#if __cplusplus >= 202302L
#define ATTRIBUTE_ASSUME(expr) [[assume(expr)]]
#elif CONFIG_GNUC_PREREQ(13, 0) && !defined(__clang__)
#define ATTRIBUTE_ASSUME(expr) __attribute__((assume(expr)))
#elif defined(__clang__) && CONFIG_HAS_BUILTIN(__builtin_assume)
// Side effect of expr is discarded
#define ATTRIBUTE_ASSUME(expr) __builtin_assume(expr)
#elif defined(_MSC_VER)
#define ATTRIBUTE_ASSUME(expr) __assume(expr)
#else
#define ATTRIBUTE_ASSUME(expr)
#endif

/* __builtin_expect is in gcc 3.0 */
#if CONFIG_GNUC_PREREQ(3, 0) && CONFIG_HAS_BUILTIN(__builtin_expect)

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

/* Copied from the cdefs.h from the glibc */
#if !(defined(__GNUC__) || defined(__clang__))
#define __attribute__(anything)
#endif

#if CONFIG_GNUC_PREREQ(2, 6) || CONFIG_HAS_ATTRIBUTE(__const__)
#define ATTRIBUTE_CONST __attribute__((__const__))
#else
#define ATTRIBUTE_CONST
#endif

#if CONFIG_GNUC_PREREQ(2, 7) || CONFIG_HAS_ATTRIBUTE(__unused__)
#define ATTRIBUTE_MAYBE_UNUSED __attribute__((__unused__))
#else
#define ATTRIBUTE_MAYBE_UNUSED
#endif

#if CONFIG_GNUC_PREREQ(2, 96) || CONFIG_HAS_ATTRIBUTE(__pure__)
#define ATTRIBUTE_PURE __attribute__((__pure__))
#else
#define ATTRIBUTE_PURE
#endif

#if CONFIG_GNUC_PREREQ(3, 0)
#define ATTRIBUTE_NOINLINE __attribute__((__noinline__))
#else
#define ATTRIBUTE_NOINLINE
#endif

#if CONFIG_GNUC_PREREQ(3, 2) || CONFIG_HAS_ATTRIBUTE(__always_inline__)
#define ATTRIBUTE_ALWAYS_INLINE __attribute__((__always_inline__))
#else
#define ATTRIBUTE_ALWAYS_INLINE
#endif

#if CONFIG_GNUC_PREREQ(4, 3) || CONFIG_HAS_ATTRIBUTE(__cold__)
#define ATTRIBUTE_COLD __attribute__((__cold__))
#else
#define ATTRIBUTE_COLD
#endif

/**
 *  mode in { "read_only", "read_only", "read_write", "write_only", "none" },
 *  mode "none" is valid iff CONFIG_GNUC_PREREQ(11, 0)
 *
 *  memory_argument_pos >= 1
 *  range_size_argument_pos >= 1
 */
#if CONFIG_GNUC_PREREQ(10, 0)
#define ATTRIBUTE_ACCESS(mode, memory_argument_pos, range_size_argument_pos) \
    __attribute__((access(mode, memory_argument_pos, range_size_argument_pos)))
#else
#define ATTRIBUTE_ACCESS(mode, memory_argument_pos, range_size_argument_pos)
#endif

#endif  // !CONFIG_MACROS_HPP
