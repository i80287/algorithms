#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L && \
    CONFIG_HAS_INCLUDE(<source_location>)
#define MISC_GET_TYPENAME_HAS_SOURCE_LOCATION
#include <source_location>
#endif

// https://en.cppreference.com/w/cpp/language/consteval
#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
#define MISC_GET_TYPENAME_CONSTEVAL consteval
#else
#define MISC_GET_TYPENAME_CONSTEVAL constexpr
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

namespace misc {

namespace misc_detail {

using std::size_t;
using std::string_view;

[[nodiscard]]
MISC_GET_TYPENAME_CONSTEVAL size_t get_typename_end_pos_impl(const string_view s) noexcept {
    // Variables are not inside of the for init for
    //  the compatibility with C++17.
    size_t opened_square_brackets = 0;
    size_t opened_round_brackets = 0;
    size_t opened_curly_brackets = 0;
    size_t opened_triangle_brackets = 0;
    size_t i = 0;
    for (const char c : s) {
        switch (static_cast<unsigned char>(c)) {
            case '(': {
                opened_round_brackets++;
                break;
            }
            case ')': {
                if (opened_round_brackets == 0) {
                    return i;
                }
                opened_round_brackets--;
                break;
            }
            case '{': {
                opened_curly_brackets++;
                break;
            }
            case '}': {
                if (opened_round_brackets == 0) {
                    return i;
                }
                opened_round_brackets--;
                break;
            }
            case '[': {
                opened_square_brackets++;
                break;
            }
            case ']': {
                if (opened_square_brackets == 0) {
                    return i;
                }
                opened_square_brackets--;
                break;
            }
            case '<': {
                opened_triangle_brackets++;
                break;
            }
            case '>': {
                if (opened_triangle_brackets == 0) {
                    return i;
                }
                opened_triangle_brackets--;
                break;
            }
            case ',':
            case ';': {
                if (opened_square_brackets == 0 && opened_round_brackets == 0 &&
                    opened_curly_brackets == 0 && opened_triangle_brackets == 0) {
                    return i;
                }
                break;
            }
            default:
                break;
        }

        i++;
    }

    return s.size();
}

#if CONFIG_GNUC_AT_LEAST(12, 1) || defined(__clang__)
#define CONSTEVAL_ASSERT(expr)                         \
    do {                                               \
        static_cast<void>(bool{(expr)} ? 0 : throw 0); \
    } while (false)
#else
#define CONSTEVAL_ASSERT(expr)                                    \
    do {                                                          \
        static_cast<void>(static_cast<bool>(expr) ? 0 : throw 0); \
    } while (false)
#endif

[[nodiscard]]
MISC_GET_TYPENAME_CONSTEVAL string_view extract_typename_impl(const string_view function_name) {
    const auto is_space = [](const char c) constexpr noexcept {
        switch (static_cast<unsigned char>(c)) {
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ':
                return true;
            default:
                return false;
        }
    };

#if defined(__GNUG__) || defined(__clang__)
    constexpr string_view type_prefix = "T = ";
    const size_t prefix_start_pos = function_name.find(type_prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != string_view::npos);
    size_t typename_start_pos = prefix_start_pos + type_prefix.size();
#elif defined(_MSC_VER)
    constexpr string_view type_prefix = "get_typename_impl<";
    const size_t prefix_start_pos = function_name.find(type_prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != string_view::npos);
    size_t typename_start_pos = prefix_start_pos + type_prefix.size();
    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    string_view piece = function_name.substr(typename_start_pos);
    constexpr string_view kKeywords[] = {
        "class",
        "struct",
        "enum",
        "union",
    };
    for (bool continue_cut = true; continue_cut;) {
        continue_cut = false;
        while (is_space(piece.front())) {
            piece.remove_prefix(1);
            typename_start_pos++;
        }
        for (const string_view keyword : kKeywords) {
#if CONFIG_HAS_AT_LEAST_CXX_20
            if (piece.starts_with(keyword))
#else
            if (keyword.size() <= piece.size() && keyword == piece.substr(0, keyword.size()))
#endif
            {
                piece.remove_prefix(keyword.size());
                typename_start_pos += keyword.size();
                continue_cut = true;
                break;
            }
        }
    }

#else
// cppcheck-suppress [preprocessorErrorDirective]
#error Unsupported compiler
#endif

    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    while (is_space(function_name[typename_start_pos])) {
        typename_start_pos++;
    }
    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    const size_t typename_end_pos =
        typename_start_pos + get_typename_end_pos_impl(function_name.substr(typename_start_pos));
    CONSTEVAL_ASSERT(typename_end_pos < function_name.size());
    CONSTEVAL_ASSERT(typename_start_pos < typename_end_pos);
    return function_name.substr(typename_start_pos, typename_end_pos - typename_start_pos);
}

template <class T>
[[nodiscard]]
MISC_GET_TYPENAME_CONSTEVAL string_view get_typename_impl() {
    const string_view function_name =
#ifdef MISC_GET_TYPENAME_HAS_SOURCE_LOCATION
        std::source_location::current().function_name();
#else
        CONFIG_CURRENT_FUNCTION_NAME;
#endif

    return ::misc::misc_detail::extract_typename_impl(function_name);
}

// clang-format off

[[nodiscard]]
MISC_GET_TYPENAME_CONSTEVAL
string_view extract_enum_value_name_impl(const string_view function_name) {
    // clang-format on

#if defined(__GNUG__) || defined(__clang__)
    constexpr string_view prefix = "EnumValue = ";
#elif defined(_MSC_VER)
    constexpr string_view prefix = "get_enum_value_name_impl<";
#else
// cppcheck-suppress [preprocessorErrorDirective]
#error Unsupported compiler
#endif

    const size_t prefix_start_pos = function_name.find(prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != string_view::npos);
    size_t value_start_pos = prefix_start_pos + prefix.size();
    CONSTEVAL_ASSERT(value_start_pos < function_name.size());
    const size_t value_end_pos =
        value_start_pos + get_typename_end_pos_impl(function_name.substr(value_start_pos));
    CONSTEVAL_ASSERT(value_start_pos < value_end_pos);
    CONSTEVAL_ASSERT(value_end_pos < function_name.size());
    string_view full_name = function_name.substr(value_start_pos, value_end_pos - value_start_pos);
    constexpr string_view kScopeResolutionOperator = "::";
    if (const size_t scope_res_operator_pos = full_name.rfind(kScopeResolutionOperator);
        scope_res_operator_pos != string_view::npos) {
        full_name = full_name.substr(scope_res_operator_pos + kScopeResolutionOperator.size());
    }
    return full_name;
}

template <auto EnumValue, class EnumType>
[[nodiscard]]
MISC_GET_TYPENAME_CONSTEVAL string_view get_enum_value_name_impl() {
    const string_view function_name =
#ifdef MISC_GET_TYPENAME_HAS_SOURCE_LOCATION
        std::source_location::current().function_name();
#else
        CONFIG_CURRENT_FUNCTION_NAME;
#endif
    return ::misc::misc_detail::extract_enum_value_name_impl(function_name);
}

#undef CONSTEVAL_ASSERT

}  // namespace misc_detail

template <class T>
ATTRIBUTE_NODISCARD_WITH_MESSAGE("requested name of the type should be used")
MISC_GET_TYPENAME_CONSTEVAL std::string_view get_typename() {
    constexpr std::string_view kTypename = ::misc::misc_detail::get_typename_impl<T>();
    return kTypename;
}

template <auto EnumValue>
ATTRIBUTE_NODISCARD_WITH_MESSAGE("requested name of the enum value should be used")
MISC_GET_TYPENAME_CONSTEVAL std::string_view get_enum_value_name() {
    using EnumType = decltype(EnumValue);
    static_assert(std::is_enum_v<EnumType>, "Value of the enum is expected");

    constexpr std::string_view kEnumValueName =
        ::misc::misc_detail::get_enum_value_name_impl<EnumValue, EnumType>();
    return kEnumValueName;
}

}  // namespace misc

#undef MISC_GET_TYPENAME_CONSTEVAL
#ifdef MISC_GET_TYPENAME_HAS_SOURCE_LOCATION
#undef MISC_GET_TYPENAME_HAS_SOURCE_LOCATION
#endif
