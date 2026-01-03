#pragma once

#include <type_traits>

#include "config_macros.hpp"

/**
 *
 * Public macros:
 *   GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)
 *     Defines bitwise operators ~, |, &, ^, |=, &=, ^=, <<, >>, <<=, >>=
 *     for the given enum type enum_type
 *
 *   GENERATE_ENUM_PLUS_MINUS_OPERATIONS(enum_type)
 *     Defines arithmetic operators +, -, +=, -=
 *     for the given enum type enum_type
 *
 *   GENERATE_ENUM_TO_INTEGER(enum_type)
 *     Defines `IntType to_integer<IntType>(enum_type)` conversion function
 *     Defines `std::underlying_type_t<enum_type> to_underlying(enum_type)` conversion function
 *
 *   GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(enum_type, ...)
 *     Defines to_string(enum_type) and to_string_view(enum_type) noexcept
 *     for the given enumeration values (members) of the enum_type
 *
 */

#if CONFIG_HAS_AT_LEAST_CXX_20
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ constexpr
#else
#define HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__ inline
#endif

#if CONFIG_HAS_AT_LEAST_CXX_17

#define HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type) static_assert(std::is_enum_v<enum_type>, "Enum type is expected")

#define HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) std::underlying_type_t<enum_type>

#else

#define HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type) static_assert(std::is_enum<enum_type>::value, "Enum type is expected")

#define HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) typename std::underlying_type<enum_type>::type

#endif

// clang-format off

#define GENERATE_ENUM_TO_INTEGER(enum_type)                                                                  \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                                               \
    template <typename IntegerType = HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type)>                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                                  \
    ATTRIBUTE_CONST                                                                                          \
    ATTRIBUTE_NODISCARD                                                                                      \
    constexpr IntegerType to_integer(const enum_type value) noexcept {                                       \
        return static_cast<IntegerType>(static_cast<HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type)>(value)); \
    }                                                                                                        \
    ATTRIBUTE_ALWAYS_INLINE                                                                                  \
    ATTRIBUTE_CONST                                                                                          \
    ATTRIBUTE_NODISCARD                                                                                      \
    constexpr HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) to_underlying(const enum_type value) noexcept { \
        return to_integer<>(value);                                                                          \
    }

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS(enum_type)                                                    \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                                          \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator~(const enum_type value) noexcept {                                     \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(~static_cast<enum_int_type__>(value));                            \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator|(const enum_type lhs, const enum_type rhs) noexcept {                  \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) |                               \
                                      static_cast<enum_int_type__>(rhs));                               \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator&(const enum_type lhs, const enum_type rhs) noexcept {                  \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) &                               \
                                      static_cast<enum_int_type__>(rhs));                               \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator^(const enum_type lhs, const enum_type rhs) noexcept {                  \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) ^                               \
                                      static_cast<enum_int_type__>(rhs));                               \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_NODISCARD                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                          \
    enum_type& operator|=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept {      \
        return lhs = lhs | rhs;                                                                         \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_NODISCARD                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                          \
    enum_type& operator&=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept {      \
        return lhs = lhs & rhs;                                                                         \
    }                                                                                                   \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_NODISCARD                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                          \
    enum_type& operator^=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept {      \
        return lhs = lhs ^ rhs;                                                                         \
    }                                                                                                   \
    template <typename IntType>                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator<<(const enum_type lhs, const IntType rhs_shift) noexcept {             \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) << rhs_shift);                  \
    }                                                                                                   \
    template <typename IntType>                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_CONST                                                                                     \
    ATTRIBUTE_NODISCARD                                                                                 \
    constexpr enum_type operator>>(const enum_type lhs, const IntType rhs_shift) noexcept {             \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                         \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) >> rhs_shift);                  \
    }                                                                                                   \
    template <typename IntType>                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_NODISCARD                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                          \
    enum_type& operator<<=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const IntType rhs_shift) noexcept { \
        return lhs = lhs << rhs_shift;                                                                  \
    }                                                                                                   \
    template <typename IntType>                                                                         \
    ATTRIBUTE_ALWAYS_INLINE                                                                             \
    ATTRIBUTE_NODISCARD                                                                                 \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                          \
    enum_type& operator>>=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const IntType rhs_shift) noexcept { \
        return lhs = lhs >> rhs_shift;                                                                  \
    }

#define GENERATE_ENUM_PLUS_MINUS_OPERATIONS(enum_type)                                             \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                                     \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator+(const enum_type lhs, const enum_type rhs) noexcept {             \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                    \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) +                          \
                                      static_cast<enum_int_type__>(rhs));                          \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_CONST                                                                                \
    ATTRIBUTE_NODISCARD                                                                            \
    constexpr enum_type operator-(const enum_type lhs, const enum_type rhs) noexcept {             \
        typedef HELPER_ENUM_UTILS_UNDERLYING_TYPE__(enum_type) enum_int_type__;                    \
        return static_cast<enum_type>(static_cast<enum_int_type__>(lhs) -                          \
                                      static_cast<enum_int_type__>(rhs));                          \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_NODISCARD                                                                            \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                     \
    enum_type& operator+=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept { \
        return lhs = lhs + rhs;                                                                    \
    }                                                                                              \
    ATTRIBUTE_ALWAYS_INLINE                                                                        \
    ATTRIBUTE_NODISCARD                                                                            \
    HELPER_ENUM_FLAG_INLINE_BIN_OP_CONSTEXPR__                                                     \
    enum_type& operator-=(enum_type& lhs ATTRIBUTE_LIFETIME_BOUND, const enum_type rhs) noexcept { \
        return lhs = lhs - rhs;                                                                    \
    }

// clang-format on

#if CONFIG_HAS_AT_LEAST_CXX_17

#include <optional>
#include <string>
#include <string_view>

#include "get_typename.hpp"

namespace enum_utils_detail {

template <typename EnumType, EnumType EnumerationValue, EnumType... EnumerationValues>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr std::string_view enum_value_to_string_view_impl(const EnumType value) noexcept {
    if (value == EnumerationValue) {
        return misc::get_enum_value_name<EnumerationValue>();
    } else if constexpr (sizeof...(EnumerationValues) > 0) {
        return enum_utils_detail::enum_value_to_string_view_impl<EnumType, EnumerationValues...>(value);
    } else {
        return "";
    }
}

template <typename EnumType, EnumType... EnumerationValues>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr std::string_view enum_value_to_string_view(const EnumType value) noexcept {
    static_assert(sizeof...(EnumerationValues) > 0, "At least one enum enumeration is expected");
    return enum_utils_detail::enum_value_to_string_view_impl<EnumType, EnumerationValues...>(value);
}

template <typename EnumType, EnumType... EnumerationValues>
[[nodiscard]] constexpr bool all_unique() noexcept {
    constexpr auto kEnumValuesSize = sizeof...(EnumerationValues);
    constexpr EnumType kEnumValues[kEnumValuesSize] = {EnumerationValues...};

    // O(n^2) instead of hash/sort because kEnumValuesSize is likely to be < 20

    // Use decltype instead of size_t in order to avoid including <cstddef>
    for (std::remove_const_t<decltype(kEnumValuesSize)> i = 0; i < kEnumValuesSize; i++) {
        for (decltype(i) j = i + 1; j < kEnumValuesSize; j++) {
            if (kEnumValues[i] == kEnumValues[j]) {
                return false;
            }
        }
    }

    return true;
}

template <typename EnumType, EnumType EnumerationValue, EnumType... EnumerationValues>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr std::optional<EnumType> try_from_string_impl(const std::string_view s) noexcept {
    if (s == misc::get_enum_value_name<EnumerationValue>()) {
        return EnumerationValue;
    } else if constexpr (sizeof...(EnumerationValues) > 0) {
        return try_from_string_impl<EnumType, EnumerationValues...>(s);
    } else {
        return std::nullopt;
    }
}

template <typename EnumType, EnumType... EnumerationValues>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr std::optional<EnumType> try_from_string(const std::string_view s) noexcept {
    static_assert(sizeof...(EnumerationValues) > 0, "At least one enum enumeration is expected");
    return enum_utils_detail::try_from_string_impl<EnumType, EnumerationValues...>(s);
}

}  // namespace enum_utils_detail

// clang-format off

#define GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(enum_type, ...)                                       \
    HELPER_ENUM_UTILS_CHECK_ENUM__(enum_type);                                                         \
    static_assert(                                                                                     \
        enum_utils_detail::all_unique<enum_type, __VA_ARGS__>(),                                       \
        "enum members passed to the GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS should be unique");       \
                                                                                                       \
    ATTRIBUTE_CONST [[nodiscard]]                                                                      \
    constexpr std::string_view to_string_view(const enum_type value) noexcept {                        \
        return enum_utils_detail::enum_value_to_string_view<enum_type, __VA_ARGS__>(value);            \
    }                                                                                                  \
                                                                                                       \
    [[nodiscard]] inline std::string to_string(const enum_type value) {                                \
        return std::string{to_string_view(value)};                                                     \
    }                                                                                                  \
                                                                                                       \
    template <typename EnumType>                                                                       \
    ATTRIBUTE_CONST [[nodiscard]]                                                                      \
    constexpr std::optional<EnumType> try_from_string(const std::string_view) noexcept;                \
                                                                                                       \
                                                                                                       \
    template <>                                                                                        \
    ATTRIBUTE_CONST [[nodiscard]]                                                                      \
    constexpr std::optional<enum_type> try_from_string<enum_type>(const std::string_view s) noexcept { \
        return enum_utils_detail::try_from_string<enum_type, __VA_ARGS__>(s);                          \
    }                                                                                                  \
                                                                                                       \
    [[nodiscard]]                                                                                      \
    constexpr bool try_from_string(const std::string_view s, enum_type& value) noexcept {              \
        const std::optional<enum_type> opt_value = try_from_string<enum_type>(s);                      \
        if (likely(opt_value.has_value())) {                                                           \
            value = *opt_value;                                                                        \
            return true;                                                                               \
        }                                                                                              \
                                                                                                       \
        return false;                                                                                  \
    }

// clang-format on

#endif
