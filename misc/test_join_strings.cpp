#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>

#include "join_strings.hpp"

#define MAKE_JOIN_STRINGS_TESTS_SUITE(CHAR_TYPE)                                                   \
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

namespace {

void test_basic_joins() {
#define STR_LITERAL(expr) expr
    MAKE_JOIN_STRINGS_TESTS_SUITE(char);
#undef STR_LITERAL

#define STR_LITERAL(expr) L##expr
    MAKE_JOIN_STRINGS_TESTS_SUITE(wchar_t);
#undef STR_LITERAL

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

#define STR_LITERAL(expr) u8##expr
    MAKE_JOIN_STRINGS_TESTS_SUITE(char8_t);
#undef STR_LITERAL

#endif

#define STR_LITERAL(expr) u##expr
    MAKE_JOIN_STRINGS_TESTS_SUITE(char16_t);
#undef STR_LITERAL

#define STR_LITERAL(expr) U##expr
    MAKE_JOIN_STRINGS_TESTS_SUITE(char32_t);
#undef STR_LITERAL
}

enum class E1 : std::uint8_t {
    kValue1 = 2,
    kValue2 = 4,
};
enum struct Condition : bool {
    kNo  = false,
    kYes = true,
};

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_ENUM_TO_STRING

namespace some {

enum class SomeEnum1 : unsigned {
    kSomeValue1 = 100,
    kSomeValue2 = 200,
};

enum class SomeEnum2 : std::int64_t {
    kSomeValue1 = -1,
    kSomeValue2 = -2,
    kSomeValue3 = -3,
};

template <class CharType = char>
[[nodiscard]] std::basic_string<CharType> to_basic_string(const SomeEnum1 e) {
    switch (e) {
        case SomeEnum1::kSomeValue1: {
            if constexpr (std::is_same_v<CharType, char>) {
                return "kSomeValue1";
            } else if constexpr (std::is_same_v<CharType, wchar_t>) {
                return L"kSomeValue1";
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
            } else if constexpr (std::is_same_v<CharType, char8_t>) {
                return u8"kSomeValue1";
#endif
            } else if constexpr (std::is_same_v<CharType, char16_t>) {
                return u"kSomeValue1";
            } else if constexpr (std::is_same_v<CharType, char32_t>) {
                return U"kSomeValue1";
            } else {
                static_assert([]() constexpr { return false; }(), "implementation error");
            }
        }
        case SomeEnum1::kSomeValue2: {
            if constexpr (std::is_same_v<CharType, char>) {
                return "kSomeValue2";
            } else if constexpr (std::is_same_v<CharType, wchar_t>) {
                return L"kSomeValue2";
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
            } else if constexpr (std::is_same_v<CharType, char8_t>) {
                return u8"kSomeValue2";
#endif
            } else if constexpr (std::is_same_v<CharType, char16_t>) {
                return u"kSomeValue2";
            } else if constexpr (std::is_same_v<CharType, char32_t>) {
                return U"kSomeValue2";
            } else {
                static_assert([]() constexpr { return false; }(), "implementation error");
            }
        }
        default: {
            if constexpr (std::is_same_v<CharType, char>) {
                return "Unknown SomeEnum1 value";
            } else if constexpr (std::is_same_v<CharType, wchar_t>) {
                return L"Unknown SomeEnum1 value";
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
            } else if constexpr (std::is_same_v<CharType, char8_t>) {
                return u8"Unknown SomeEnum1 value";
#endif
            } else if constexpr (std::is_same_v<CharType, char16_t>) {
                return u"Unknown SomeEnum1 value";
            } else if constexpr (std::is_same_v<CharType, char32_t>) {
                return U"Unknown SomeEnum1 value";
            } else {
                static_assert([]() constexpr { return false; }(), "implementation error");
            }
        }
    }
}

[[nodiscard]] std::string to_string(const SomeEnum2 e) {
    switch (e) {
        case SomeEnum2::kSomeValue1: {
            return "kSomeValue1";
        }
        case SomeEnum2::kSomeValue2: {
            return "kSomeValue2";
        }
        case SomeEnum2::kSomeValue3: {
            return "kSomeValue3";
        }
        default: {
            return "Unknown SomeEnum2 value";
        }
    }
}

}  // namespace some

void test_custom_enum_to_string() {
    // clang-format off

    assert(misc::JoinStrings(some::SomeEnum2{}) == to_string(some::SomeEnum2{}));
    assert(misc::JoinStrings(some::SomeEnum2::kSomeValue1) == to_string(some::SomeEnum2::kSomeValue1));
    assert(misc::JoinStrings(some::SomeEnum2::kSomeValue2) == to_string(some::SomeEnum2::kSomeValue2));
    assert(misc::JoinStrings(some::SomeEnum2::kSomeValue3) == to_string(some::SomeEnum2::kSomeValue3));


    assert(misc::JoinStrings(some::SomeEnum1{}) == to_basic_string(some::SomeEnum1{}));
    assert(misc::JoinStrings(some::SomeEnum1::kSomeValue1) == to_basic_string(some::SomeEnum1::kSomeValue1));
    assert(misc::JoinStrings(some::SomeEnum1::kSomeValue2) == to_basic_string(some::SomeEnum1::kSomeValue2));

    assert(misc::JoinStrings<wchar_t>(some::SomeEnum1{}) == to_basic_string<wchar_t>(some::SomeEnum1{}));
    assert(misc::JoinStrings<wchar_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<wchar_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::JoinStrings<wchar_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<wchar_t>(some::SomeEnum1::kSomeValue2));

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    assert(misc::JoinStrings<char8_t>(some::SomeEnum1{}) == to_basic_string<char8_t>(some::SomeEnum1{}));
    assert(misc::JoinStrings<char8_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::JoinStrings<char8_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue2));
#endif

    assert(misc::JoinStrings<char16_t>(some::SomeEnum1{}) == to_basic_string<char16_t>(some::SomeEnum1{}));
    assert(misc::JoinStrings<char16_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::JoinStrings<char16_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue2));

    assert(misc::JoinStrings<char32_t>(some::SomeEnum1{}) == to_basic_string<char32_t>(some::SomeEnum1{}));
    assert(misc::JoinStrings<char32_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::JoinStrings<char32_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue2));

    // clang-format on
}

#endif

void test_enums() {
    assert(misc::JoinStrings(E1::kValue1) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::JoinStrings<wchar_t>(E1::kValue1) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::JoinStrings(E1::kValue2) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));
    assert(misc::JoinStrings<wchar_t>(E1::kValue2) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_ENUM_TO_STRING
    test_custom_enum_to_string();
#endif
}

void test_pointers() {
    // clang-format off

    struct S {
        static void static_method() {}
        static void noexcept_static_method() noexcept {}
    };

    assert(misc::JoinStrings(nullptr) == "null");
    assert(misc::JoinStrings(std::nullptr_t{}) == "null");

    const S s{};

    assert(misc::JoinStrings(static_cast<const void*>(0)) == "null");
    assert(misc::JoinStrings(static_cast<const void*>(nullptr)) == "null");
    assert(misc::JoinStrings(static_cast<const void*>(&s)) == std::to_string(reinterpret_cast<uintptr_t>(&s)));
    assert(misc::JoinStrings(&s) == std::to_string(reinterpret_cast<uintptr_t>(&s)));
    assert(misc::JoinStrings(&test_basic_joins) == std::to_string(reinterpret_cast<uintptr_t>(&test_basic_joins)));
    assert(misc::JoinStrings(&test_enums) == std::to_string(reinterpret_cast<uintptr_t>(&test_enums)));
    assert(misc::JoinStrings(&test_pointers) == std::to_string(reinterpret_cast<uintptr_t>(&test_pointers)));

    assert(misc::JoinStrings(&S::static_method) == std::to_string(reinterpret_cast<uintptr_t>(&S::static_method)));
    assert(misc::JoinStrings(&S::noexcept_static_method) == std::to_string(reinterpret_cast<uintptr_t>(&S::noexcept_static_method)));

    // clang-format on
}

}  // namespace

int main() {
    test_basic_joins();
    test_enums();
    test_pointers();
}
