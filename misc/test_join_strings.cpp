#include <cassert>
#include <string>
#include <string_view>

#include "join_strings.hpp"

#define MAKE_JOIN_STRINGS_TEST_BLOCK(CHAR_TYPE)                                                    \
    do {                                                                                           \
        assert(misc::JoinStrings(STR_LITERAL("")) == STR_LITERAL(""));                             \
        assert(misc::JoinStrings(STR_LITERAL("ab"), STR_LITERAL("cde")) == STR_LITERAL("abcde"));  \
        assert(misc::JoinStrings(STR_LITERAL("ab"), STR_LITERAL("cde"), STR_LITERAL("fghi")) ==    \
               STR_LITERAL("abcdefghi"));                                                          \
        assert(misc::JoinStrings(STR_LITERAL("ab"), STR_LITERAL("cde"), STR_LITERAL("fghi"),       \
                                 STR_LITERAL("jklmn")) == STR_LITERAL("abcdefghijklmn"));          \
        assert(misc::JoinStrings(STR_LITERAL("ab"), 1, STR_LITERAL("cde"), 2, STR_LITERAL("fghi"), \
                                 3, STR_LITERAL("jklmn")) == STR_LITERAL("ab1cde2fghi3jklmn"));    \
                                                                                                   \
        const std::basic_string s1      = STR_LITERAL("ab");                                       \
        const std::basic_string_view s2 = STR_LITERAL("cde");                                      \
        const auto s3                   = STR_LITERAL("fghi");                                     \
        const auto* const s4            = STR_LITERAL("jklmn");                                    \
                                                                                                   \
        assert(misc::JoinStrings(s1) == s1);                                                       \
        assert(misc::JoinStrings(s2) == s2);                                                       \
        assert(misc::JoinStrings(s3) == s3);                                                       \
        assert(misc::JoinStrings(s4) == s4);                                                       \
                                                                                                   \
        assert(misc::JoinStrings(s1, 1, STR_LITERAL(""), s2, 2, STR_LITERAL(""), s3,               \
                                 STR_LITERAL(""), 3, s4) == STR_LITERAL("ab1cde2fghi3jklmn"));     \
        assert(misc::JoinStrings(s2, 1, STR_LITERAL(""), s3, 2, STR_LITERAL(""), s4,               \
                                 STR_LITERAL(""), 3, s1) == STR_LITERAL("cde1fghi2jklmn3ab"));     \
        assert(misc::JoinStrings(s3, 1, STR_LITERAL(""), s4, 2, STR_LITERAL(""), s1,               \
                                 STR_LITERAL(""), 3, s2) == STR_LITERAL("fghi1jklmn2ab3cde"));     \
        assert(misc::JoinStrings(s4, 1, STR_LITERAL(""), s1, 2, STR_LITERAL(""), s2,               \
                                 STR_LITERAL(""), 3, s3) == STR_LITERAL("jklmn1ab2cde3fghi"));     \
                                                                                                   \
        assert(misc::JoinStrings(0, s1, STR_LITERAL(""), 1, s2, STR_LITERAL(""), 2,                \
                                 STR_LITERAL(""), s3, 3,                                           \
                                 s4) == STR_LITERAL("0ab1cde2fghi3jklmn"));                        \
        assert(misc::JoinStrings(0, s2, STR_LITERAL(""), 1, s3, STR_LITERAL(""), 2,                \
                                 STR_LITERAL(""), s4, 3,                                           \
                                 s1) == STR_LITERAL("0cde1fghi2jklmn3ab"));                        \
        assert(misc::JoinStrings(0, s3, STR_LITERAL(""), 1, s4, STR_LITERAL(""), 2,                \
                                 STR_LITERAL(""), s1, 3,                                           \
                                 s2) == STR_LITERAL("0fghi1jklmn2ab3cde"));                        \
        assert(misc::JoinStrings(0, s4, STR_LITERAL(""), 1, s1, STR_LITERAL(""), 2,                \
                                 STR_LITERAL(""), s2, 3,                                           \
                                 s3) == STR_LITERAL("0jklmn1ab2cde3fghi"));                        \
                                                                                                   \
        assert(misc::JoinStrings(s1, 1, s2, 2, s3, 3, s4) == STR_LITERAL("ab1cde2fghi3jklmn"));    \
        assert(misc::JoinStrings(s2, 1, s3, 2, s4, 3, s1) == STR_LITERAL("cde1fghi2jklmn3ab"));    \
        assert(misc::JoinStrings(s3, 1, s4, 2, s1, 3, s2) == STR_LITERAL("fghi1jklmn2ab3cde"));    \
        assert(misc::JoinStrings(s4, 1, s1, 2, s2, 3, s3) == STR_LITERAL("jklmn1ab2cde3fghi"));    \
                                                                                                   \
        assert(misc::JoinStrings(0, s1, 1, s2, 2, s3, 3, s4) ==                                    \
               STR_LITERAL("0ab1cde2fghi3jklmn"));                                                 \
        assert(misc::JoinStrings(0, s2, 1, s3, 2, s4, 3, s1) ==                                    \
               STR_LITERAL("0cde1fghi2jklmn3ab"));                                                 \
        assert(misc::JoinStrings(0, s3, 1, s4, 2, s1, 3, s2) ==                                    \
               STR_LITERAL("0fghi1jklmn2ab3cde"));                                                 \
        assert(misc::JoinStrings(0, s4, 1, s1, 2, s2, 3, s3) ==                                    \
               STR_LITERAL("0jklmn1ab2cde3fghi"));                                                 \
                                                                                                   \
        assert(misc::JoinStrings<CHAR_TYPE>(1) == STR_LITERAL("1"));                               \
        assert(misc::JoinStrings<CHAR_TYPE>(1, 2) == STR_LITERAL("12"));                           \
        assert(misc::JoinStrings<CHAR_TYPE>(1, 2, 3) == STR_LITERAL("123"));                       \
        assert(misc::JoinStrings<CHAR_TYPE>(1, 2, 3, 4) == STR_LITERAL("1234"));                   \
        assert(misc::JoinStrings<CHAR_TYPE>(1, 2, 3, 4, 5) == STR_LITERAL("12345"));               \
    } while (false)

int main() {
#define STR_LITERAL(expr) expr
    MAKE_JOIN_STRINGS_TEST_BLOCK(char);
#undef STR_LITERAL

#define STR_LITERAL(expr) L##expr
    MAKE_JOIN_STRINGS_TEST_BLOCK(wchar_t);
#undef STR_LITERAL

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

#define STR_LITERAL(expr) u8##expr
    MAKE_JOIN_STRINGS_TEST_BLOCK(char8_t);
#undef STR_LITERAL

#endif

#define STR_LITERAL(expr) u##expr
    MAKE_JOIN_STRINGS_TEST_BLOCK(char16_t);
#undef STR_LITERAL

#define STR_LITERAL(expr) U##expr
    MAKE_JOIN_STRINGS_TEST_BLOCK(char32_t);
#undef STR_LITERAL
}
