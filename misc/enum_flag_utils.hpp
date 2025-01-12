#pragma once

#include <type_traits>

#include "config_macros.hpp"

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)                                   \
    static_assert(std::is_enum<enum_type>::value, "Enum type is expected");            \
    ATTRIBUTE_ALWAYS_INLINE                                                            \
    ATTRIBUTE_CONST                                                                    \
    ATTRIBUTE_NODISCARD                                                                \
    constexpr enum_type operator~(const enum_type value) noexcept {                    \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;        \
        return static_cast<enum_type>(~static_cast<enum_int_type__>(value));           \
    }                                                                                  \
    ATTRIBUTE_ALWAYS_INLINE                                                            \
    ATTRIBUTE_CONST                                                                    \
    ATTRIBUTE_NODISCARD                                                                \
    constexpr enum_type operator|(const enum_type lhs, const enum_type rhs) noexcept { \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;        \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) |              \
                                      static_cast<enum_int_type__>(rhs));              \
    }                                                                                  \
    ATTRIBUTE_ALWAYS_INLINE                                                            \
    ATTRIBUTE_CONST                                                                    \
    ATTRIBUTE_NODISCARD                                                                \
    constexpr enum_type operator&(const enum_type lhs, const enum_type rhs) noexcept { \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;        \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) &              \
                                      static_cast<enum_int_type__>(rhs));              \
    }                                                                                  \
                                                                                       \
    ATTRIBUTE_ALWAYS_INLINE                                                            \
    ATTRIBUTE_CONST                                                                    \
    ATTRIBUTE_NODISCARD                                                                \
    constexpr enum_type operator^(const enum_type lhs, const enum_type rhs) noexcept { \
        typedef typename std::underlying_type<enum_type>::type enum_int_type__;        \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) ^              \
                                      static_cast<enum_int_type__>(rhs));              \
    }
