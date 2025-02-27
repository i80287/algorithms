#pragma once

#include <type_traits>

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ constexpr
#else
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ inline
#endif

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)                                               \
    static_assert(std::is_enum<enum_type>::value, "Enum type is expected");                        \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator~(const enum_type value) noexcept {                                \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;                    \
        return static_cast<enum_type>(~static_cast<enum_int_type__>(value));                       \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator|(const enum_type lhs, const enum_type rhs) noexcept {             \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;                    \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) |                          \
                                      static_cast<enum_int_type__>(rhs));                          \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator&(const enum_type lhs, const enum_type rhs) noexcept {             \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;                    \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) &                          \
                                      static_cast<enum_int_type__>(rhs));                          \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator^(const enum_type lhs, const enum_type rhs) noexcept {             \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;                    \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) ^                          \
                                      static_cast<enum_int_type__>(rhs));                          \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_NODISCARD                                                                            \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                     \
    enum_type& operator|=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept { \
        return lhs = lhs | rhs;                                                                    \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_NODISCARD                                                                            \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                     \
    enum_type& operator&=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept { \
        return lhs = lhs & rhs;                                                                    \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_NODISCARD                                                                            \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                     \
    enum_type& operator^=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept { \
        return lhs = lhs ^ rhs;                                                                    \
    }
