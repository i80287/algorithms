#pragma once

#include "test_tools.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

#define STR_LITERAL(CharType, LITERAL)                             \
    []<class = CharType>() constexpr noexcept -> decltype(auto) {  \
        if constexpr (std::is_same_v<CharType, char>) {            \
            return LITERAL;                                        \
        } else if constexpr (std::is_same_v<CharType, wchar_t>) {  \
            return L##LITERAL;                                     \
        } else if constexpr (std::is_same_v<CharType, char8_t>) {  \
            return u8##LITERAL;                                    \
        } else if constexpr (std::is_same_v<CharType, char16_t>) { \
            return u##LITERAL;                                     \
        } else {                                                   \
            static_assert(std::is_same_v<CharType, char32_t>);     \
            return U##LITERAL;                                     \
        }                                                          \
    }()

#else

#define STR_LITERAL(CharType, LITERAL)                             \
    []() constexpr noexcept -> decltype(auto) {                    \
        if constexpr (std::is_same_v<CharType, char>) {            \
            return LITERAL;                                        \
        } else if constexpr (std::is_same_v<CharType, wchar_t>) {  \
            return L##LITERAL;                                     \
        } else if constexpr (std::is_same_v<CharType, char16_t>) { \
            return u##LITERAL;                                     \
        } else if constexpr (std::is_same_v<CharType, char32_t>) { \
            return U##LITERAL;                                     \
        }                                                          \
    }()

#endif
