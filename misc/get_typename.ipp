#ifndef GET_TYPENAME_INCLUDING_IMPLEMENTATION
// cppcheck-suppress [preprocessorErrorDirective]
#error This header should not be used directly
#endif

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L && CONFIG_HAS_INCLUDE(<source_location>)

#if !CONFIG_COMPILER_IS_MSVC ||      \
    (CONFIG_MSVC_AT_LEAST(19, 39) && \
     (!defined(_USE_DETAILED_FUNCTION_NAME_IN_SOURCE_LOCATION) || _USE_DETAILED_FUNCTION_NAME_IN_SOURCE_LOCATION))
#define MISC_GET_TYPENAME_USE_SOURCE_LOCATION_FUNCTION_NAME
#include <source_location>
#endif
#endif

namespace misc {

namespace detail {

using std::size_t;
using std::string_view;

[[nodiscard]] constexpr size_t get_typename_end_pos_impl(const string_view s) noexcept {
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
                if (opened_curly_brackets == 0) {
                    return i;
                }
                opened_curly_brackets--;
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
                if (opened_square_brackets == 0 && opened_round_brackets == 0 && opened_curly_brackets == 0 &&
                    opened_triangle_brackets == 0) {
                    return i;
                }
                break;
            }
            default: {
                break;
            }
        }

        i++;
    }

    return s.size();
}

constexpr void constexpr_assert(const bool value) noexcept {
#ifdef __clang_analyzer__
    [[clang::suppress]]  // core.DivideZero
#endif
    static_cast<void>(0 / static_cast<int>(value));  // NOLINT(clang-analyzer-core.DivideZero)
}

[[nodiscard]] constexpr string_view extract_typename_impl(const string_view function_name) {
    const auto is_space = [](const char c) constexpr noexcept {
        switch (static_cast<unsigned char>(c)) {
            case '\t':
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ': {
                return true;
            }
            default: {
                return false;
            }
        }
    };

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
    constexpr string_view type_prefix = "T = ";
    const size_t prefix_start_pos = function_name.find(type_prefix);
    ::misc::detail::constexpr_assert(prefix_start_pos != string_view::npos);
    size_t typename_start_pos = prefix_start_pos + type_prefix.size();
#elif CONFIG_COMPILER_IS_MSVC
    constexpr string_view type_prefix = "get_typename_impl<";
    const size_t prefix_start_pos = function_name.find(type_prefix);
    ::misc::detail::constexpr_assert(prefix_start_pos != string_view::npos);
    size_t typename_start_pos = prefix_start_pos + type_prefix.size();
    ::misc::detail::constexpr_assert(typename_start_pos < function_name.size());
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

    ::misc::detail::constexpr_assert(typename_start_pos < function_name.size());
    while (is_space(function_name[typename_start_pos])) {
        typename_start_pos++;
    }
    ::misc::detail::constexpr_assert(typename_start_pos < function_name.size());
    const size_t typename_end_pos =
        typename_start_pos + get_typename_end_pos_impl(function_name.substr(typename_start_pos));
    ::misc::detail::constexpr_assert(typename_end_pos < function_name.size());
    ::misc::detail::constexpr_assert(typename_start_pos < typename_end_pos);
    return function_name.substr(typename_start_pos, typename_end_pos - typename_start_pos);
}

template <class T>
[[nodiscard]] constexpr string_view get_typename_impl() {
    const string_view function_name =
#ifdef MISC_GET_TYPENAME_USE_SOURCE_LOCATION_FUNCTION_NAME
        std::source_location::current().function_name();
#else
        CONFIG_CURRENT_FUNCTION_NAME;
#endif

    return ::misc::detail::extract_typename_impl(function_name);
}

[[nodiscard]] constexpr string_view extract_enum_value_name_impl(const string_view function_name) {
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
    constexpr string_view prefix = "EnumValue = ";
#elif CONFIG_COMPILER_IS_MSVC
    constexpr string_view prefix = "get_enum_value_name_impl<";
#else
// cppcheck-suppress [preprocessorErrorDirective]
#error Unsupported compiler
#endif

    const size_t prefix_start_pos = function_name.find(prefix);
    ::misc::detail::constexpr_assert(prefix_start_pos != string_view::npos);
    size_t value_start_pos = prefix_start_pos + prefix.size();
    ::misc::detail::constexpr_assert(value_start_pos < function_name.size());
    const size_t value_end_pos = value_start_pos + get_typename_end_pos_impl(function_name.substr(value_start_pos));
    ::misc::detail::constexpr_assert(value_start_pos < value_end_pos);
    ::misc::detail::constexpr_assert(value_end_pos < function_name.size());
    string_view full_name = function_name.substr(value_start_pos, value_end_pos - value_start_pos);
    constexpr string_view kScopeResolutionOperator = "::";
    const size_t scope_res_operator_pos = full_name.rfind(kScopeResolutionOperator);
    if (scope_res_operator_pos != string_view::npos) {
        full_name = full_name.substr(scope_res_operator_pos + kScopeResolutionOperator.size());
    }

    return full_name;
}

template <auto EnumValue, class EnumType>
[[nodiscard]] constexpr string_view get_enum_value_name_impl() {
    const string_view function_name =
#ifdef MISC_GET_TYPENAME_USE_SOURCE_LOCATION_FUNCTION_NAME
        std::source_location::current().function_name();
#else
        CONFIG_CURRENT_FUNCTION_NAME;
#endif
    return ::misc::detail::extract_enum_value_name_impl(function_name);
}

[[nodiscard]] constexpr string_view unqualify_typename(const string_view type_name) noexcept {
    constexpr string_view kTemplateBeginning = "<";
    constexpr string_view kResolutionOperator = "::";

    const string_view possible_namespace_location = type_name.substr(0, type_name.find(kTemplateBeginning));
    const size_t resolution_operator_begin_pos = possible_namespace_location.rfind(kResolutionOperator);
    if (resolution_operator_begin_pos == string_view::npos) {
        return type_name;
    }

    return type_name.substr(resolution_operator_begin_pos + kResolutionOperator.size());
}

}  // namespace detail

template <class T>
constexpr std::string_view get_qualified_typename() {
    constexpr std::string_view kTypename = ::misc::detail::get_typename_impl<T>();
    return kTypename;
}

template <auto EnumValue>
constexpr std::string_view get_enum_value_name() {
    using EnumType = decltype(EnumValue);
    static_assert(std::is_enum_v<EnumType>, "Value of the enum is expected");

    constexpr std::string_view kEnumValueName = ::misc::detail::get_enum_value_name_impl<EnumValue, EnumType>();
    return kEnumValueName;
}

template <class T>
constexpr std::string_view get_unqualified_typename() {
    constexpr std::string_view kTypename = ::misc::detail::unqualify_typename(::misc::get_qualified_typename<T>());
    return kTypename;
}

}  // namespace misc

#ifdef MISC_GET_TYPENAME_USE_SOURCE_LOCATION_FUNCTION_NAME
#undef MISC_GET_TYPENAME_USE_SOURCE_LOCATION_FUNCTION_NAME
#endif
