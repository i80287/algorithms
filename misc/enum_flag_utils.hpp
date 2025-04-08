#pragma once

#include <type_traits>

#include "config_macros.hpp"

/**
 *
 * Public macros:
 *   Defines bitwise operators ~, |, &, ^, |=, &=, ^= for the given enum type enum_type:
 *     GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)
 *
 */

#if CONFIG_HAS_AT_LEAST_CXX_20
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ constexpr
#else
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ inline
#endif

#if CONFIG_HAS_AT_LEAST_CXX_17

#define HELPER_ENUM_FLAG_CHECK_ENUM__(enum_type) \
    static_assert(std::is_enum_v<enum_type>, "Enum type is expected")
#define HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type) \
    typedef std::underlying_type_t<enum_type> enum_int_type__

#else

#define HELPER_ENUM_FLAG_CHECK_ENUM__(enum_type) \
    static_assert(std::is_enum<enum_type>::value, "Enum type is expected")
#define HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type) \
    typedef typename std::underlying_type<enum_type>::type enum_int_type__

#endif

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)                                    \
    HELPER_ENUM_FLAG_CHECK_ENUM__(enum_type);                                           \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_CONST                                                                     \
    ATTRIBUTE_NODISCARD                                                                 \
    constexpr enum_type operator~(const enum_type value) CONFIG_NOEXCEPT_FUNCTION {     \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                            \
        return static_cast<enum_type>(~static_cast<enum_int_type__>(value));            \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_CONST                                                                     \
    ATTRIBUTE_NODISCARD                                                                 \
    constexpr enum_type operator|(const enum_type lhs, const enum_type rhs)             \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) |               \
                                      static_cast<enum_int_type__>(rhs));               \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_CONST                                                                     \
    ATTRIBUTE_NODISCARD                                                                 \
    constexpr enum_type operator&(const enum_type lhs, const enum_type rhs)             \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) &               \
                                      static_cast<enum_int_type__>(rhs));               \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_CONST                                                                     \
    ATTRIBUTE_NODISCARD                                                                 \
    constexpr enum_type operator^(const enum_type lhs, const enum_type rhs)             \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) ^               \
                                      static_cast<enum_int_type__>(rhs));               \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_NODISCARD                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                          \
    enum_type& operator|=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        return lhs = lhs | rhs;                                                         \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_NODISCARD                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                          \
    enum_type& operator&=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        return lhs = lhs & rhs;                                                         \
    }                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                             \
    ATTRIBUTE_NODISCARD                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                          \
    enum_type& operator^=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) \
        CONFIG_NOEXCEPT_FUNCTION {                                                      \
        return lhs = lhs ^ rhs;                                                         \
    }
