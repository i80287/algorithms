#pragma once

#include <type_traits>

#include "config_macros.hpp"

/**
 *
 * Public macros:
 *   Defines bitwise operators ~, |, &, ^, |=, &=, ^=, <<, >>, <<=, >>=
 *   for the given enum type enum_type:
 *     GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)
 *
 *   Defines `IntType to_integer<IntType>(enum_type)` conversion function
 *     GENERATE_ENUM_TO_INTEGER(enum_type)
 *
 *   Defines to_string(enum_type) and to_string_view(enum_type) noexcept
 *   for the given enumeration values (members) of the enum_type
 *     GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(enum_type, ...)
 */

#if CONFIG_HAS_AT_LEAST_CXX_20
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ constexpr
#else
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ inline
#endif

#if CONFIG_HAS_AT_LEAST_CXX_17

#define HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type) \
    static_assert(std::is_enum_v<enum_type>, "Enum type is expected")

#define HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) std::underlying_type_t<enum_type>

#else

#define HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type) \
    static_assert(std::is_enum<enum_type>::value, "Enum type is expected")

#define HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) \
    typename std::underlying_type<enum_type>::type

#endif

#define HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type) \
    typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__

// clang-format off

#define GENERATE_ENUM_TO_INTEGER(enum_type)                                            \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                         \
    template <class IntegerType = HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type)>      \
    ATTRIBUTE_ALWAYS_INLINE                                                            \
    ATTRIBUTE_CONST                                                                    \
    ATTRIBUTE_NODISCARD                                                                \
    constexpr IntegerType to_integer(const enum_type value) CONFIG_NOEXCEPT_FUNCTION { \
        return static_cast<IntegerType>(                                               \
            static_cast<HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type)>(value));       \
    }

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)                                                                    \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                                                          \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator~(const enum_type value) CONFIG_NOEXCEPT_FUNCTION {                                     \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(~static_cast<enum_int_type__>(value));                                            \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator|(const enum_type lhs, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {                  \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) |                                               \
                                      static_cast<enum_int_type__>(rhs));                                               \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator&(const enum_type lhs, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {                  \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) &                                               \
                                      static_cast<enum_int_type__>(rhs));                                               \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator^(const enum_type lhs, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {                  \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) ^                                               \
                                      static_cast<enum_int_type__>(rhs));                                               \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_NODISCARD                                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                                          \
    enum_type& operator|=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {      \
        return lhs = lhs | rhs;                                                                                         \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_NODISCARD                                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                                          \
    enum_type& operator&=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {      \
        return lhs = lhs & rhs;                                                                                         \
    }                                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_NODISCARD                                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                                          \
    enum_type& operator^=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) CONFIG_NOEXCEPT_FUNCTION {      \
        return lhs = lhs ^ rhs;                                                                                         \
    }                                                                                                                   \
    template <typename IntType>                                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator<<(const enum_type lhs, const IntType rhs_shift) CONFIG_NOEXCEPT_FUNCTION {             \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) << rhs_shift);                                  \
    }                                                                                                                   \
    template <typename IntType>                                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    constexpr enum_type operator>>(const enum_type lhs, const IntType rhs_shift) CONFIG_NOEXCEPT_FUNCTION {             \
        HELPER_ENUM_FLAG_TYPEDEF_ENUM_INT_TYPE__(enum_type);                                                            \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) >> rhs_shift);                                  \
    }                                                                                                                   \
    template <typename IntType>                                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                                          \
    enum_type& operator<<=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const IntType rhs_shift) CONFIG_NOEXCEPT_FUNCTION { \
        return lhs = lhs << rhs_shift;                                                                                  \
    }                                                                                                                   \
    template <typename IntType>                                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                                             \
    ATTRIBUTE_CONST                                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                                          \
    enum_type& operator>>=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const IntType rhs_shift) CONFIG_NOEXCEPT_FUNCTION { \
        return lhs = lhs >> rhs_shift;                                                                                  \
    }

// clang-format on

#if CONFIG_HAS_AT_LEAST_CXX_17

#include <string>
#include <string_view>

#include "get_typename.hpp"

namespace enum_utils_detail {

// clang-format off
template <class EnumType, EnumType EnumerationValue, EnumType... EnumerationValues>
[[nodiscard]]
ATTRIBUTE_CONST
ATTRIBUTE_ALWAYS_INLINE
constexpr std::string_view enum_value_to_string_view_impl(const EnumType value) noexcept {
    // clang-format on
    if (value == EnumerationValue) {
        return misc::get_enum_value_name<EnumerationValue>();
    } else if constexpr (sizeof...(EnumerationValues) > 0) {
        return enum_utils_detail::enum_value_to_string_view_impl<EnumType, EnumerationValues...>(
            value);
    } else {
        return "";
    }
}

template <class EnumType, EnumType... EnumerationValues>
ATTRIBUTE_CONST [[nodiscard]]
constexpr std::string_view enum_value_to_string_view(const EnumType value) noexcept {
    HELPER_ENUM_UTILS_CHECK_ENUM__(EnumType);
    static_assert(sizeof...(EnumerationValues) > 0, "At least one enum enumeration is expected");
    return enum_utils_detail::enum_value_to_string_view_impl<EnumType, EnumerationValues...>(value);
}

}  // namespace enum_utils_detail

#define GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(enum_type, ...)                            \
    ATTRIBUTE_CONST [[nodiscard]]                                                           \
    constexpr std::string_view to_string_view(const enum_type value) noexcept {             \
        return enum_utils_detail::enum_value_to_string_view<enum_type, __VA_ARGS__>(value); \
    }                                                                                       \
    [[nodiscard]] inline std::string to_string(const enum_type value) {                     \
        return std::string{to_string_view(value)};                                          \
    }

#endif
