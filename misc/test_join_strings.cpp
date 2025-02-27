// clang-format off
#include "join_strings.hpp"
// clang-format on

#include <cassert>
#include <cstdint>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <vector>

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

template <class CharType>
class JoinStringsTestSuite final {
public:
    static void run() {
        test_empty();
        test_misc();
        test_without_chars();
    }

private:
    static void test_empty() {
        assert(misc::JoinStrings(STR_LITERAL(CharType, "")) == STR_LITERAL(CharType, ""));
    }

    static void test_misc() {
        const std::basic_string s1      = STR_LITERAL(CharType, "ab");
        const std::basic_string_view s2 = STR_LITERAL(CharType, "cde");
        const auto s3                   = STR_LITERAL(CharType, "fghi");
        const auto* const s4            = STR_LITERAL(CharType, "jklmn");

        assert(misc::JoinStrings(s1) == s1);
        assert(misc::JoinStrings(s2) == s2);
        assert(misc::JoinStrings(s3) == s3);
        assert(misc::JoinStrings(s4) == s4);

        assert(misc::JoinStrings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde")) ==
               STR_LITERAL(CharType, "abcde"));
        assert(misc::JoinStrings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde"),
                                 STR_LITERAL(CharType, "fghi")) ==
               STR_LITERAL(CharType, "abcdefghi"));
        assert(misc::JoinStrings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde"),
                                 STR_LITERAL(CharType, "fghi"), STR_LITERAL(CharType, "jklmn")) ==
               STR_LITERAL(CharType, "abcdefghijklmn"));
        assert(misc::JoinStrings(STR_LITERAL(CharType, "ab"), 1, STR_LITERAL(CharType, "cde"), 2,
                                 STR_LITERAL(CharType, "fghi"), 3,
                                 STR_LITERAL(CharType, "jklmn")) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));

        assert(misc::JoinStrings(s1, 1, STR_LITERAL(CharType, ""), s2, 2, STR_LITERAL(CharType, ""),
                                 s3, STR_LITERAL(CharType, ""), 3,
                                 s4) == STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::JoinStrings(s2, 1, STR_LITERAL(CharType, ""), s3, 2, STR_LITERAL(CharType, ""),
                                 s4, STR_LITERAL(CharType, ""), 3,
                                 s1) == STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::JoinStrings(s3, 1, STR_LITERAL(CharType, ""), s4, 2, STR_LITERAL(CharType, ""),
                                 s1, STR_LITERAL(CharType, ""), 3,
                                 s2) == STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::JoinStrings(s4, 1, STR_LITERAL(CharType, ""), s1, 2, STR_LITERAL(CharType, ""),
                                 s2, STR_LITERAL(CharType, ""), 3,
                                 s3) == STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::JoinStrings(0, s1, STR_LITERAL(CharType, ""), 1, s2, STR_LITERAL(CharType, ""),
                                 2, STR_LITERAL(CharType, ""), s3, 3,
                                 s4) == STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::JoinStrings(0, s2, STR_LITERAL(CharType, ""), 1, s3, STR_LITERAL(CharType, ""),
                                 2, STR_LITERAL(CharType, ""), s4, 3,
                                 s1) == STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::JoinStrings(0, s3, STR_LITERAL(CharType, ""), 1, s4, STR_LITERAL(CharType, ""),
                                 2, STR_LITERAL(CharType, ""), s1, 3,
                                 s2) == STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::JoinStrings(0, s4, STR_LITERAL(CharType, ""), 1, s1, STR_LITERAL(CharType, ""),
                                 2, STR_LITERAL(CharType, ""), s2, 3,
                                 s3) == STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));

        assert(misc::JoinStrings(s1, 1, s2, 2, s3, 3, s4) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::JoinStrings(s2, 1, s3, 2, s4, 3, s1) ==
               STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::JoinStrings(s3, 1, s4, 2, s1, 3, s2) ==
               STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::JoinStrings(s4, 1, s1, 2, s2, 3, s3) ==
               STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::JoinStrings(0, s1, 1, s2, 2, s3, 3, s4) ==
               STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::JoinStrings(0, s2, 1, s3, 2, s4, 3, s1) ==
               STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::JoinStrings(0, s3, 1, s4, 2, s1, 3, s2) ==
               STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::JoinStrings(0, s4, 1, s1, 2, s2, 3, s3) ==
               STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));
    }

    static void test_without_chars() {
        assert(misc::JoinStrings<CharType>(1) == STR_LITERAL(CharType, "1"));
        assert(misc::JoinStrings<CharType>(1, 2) == STR_LITERAL(CharType, "12"));
        assert(misc::JoinStrings<CharType>(1, 2, 3) == STR_LITERAL(CharType, "123"));
        assert(misc::JoinStrings<CharType>(1, 2, 3, 4) == STR_LITERAL(CharType, "1234"));
        assert(misc::JoinStrings<CharType>(1, 2, 3, 4, 5) == STR_LITERAL(CharType, "12345"));
    }
};

namespace {

namespace join_strings_test {

void test_basic_joins() {
    JoinStringsTestSuite<char>::run();
    JoinStringsTestSuite<wchar_t>::run();
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    JoinStringsTestSuite<char8_t>::run();
#endif
    JoinStringsTestSuite<char16_t>::run();
    JoinStringsTestSuite<char32_t>::run();
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

}  // namespace join_strings_test

void test_join_strings() {
    join_strings_test::test_basic_joins();
    join_strings_test::test_enums();
    join_strings_test::test_pointers();
}

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

namespace join_strings_collection_test {

template <class CharType>
class JoinStringsCollectionTestSuit final {
public:
    static void run() {
        test_empty_collection();
        test_1_element_vec();
        test_1_element_set();
        test_3_elements_arr();
        test_list_of_empty_strings();
    }

private:
    static constexpr CharType kCharSep                          = STR_LITERAL(CharType, '~');
    static constexpr std::basic_string_view<CharType> kEmptySep = STR_LITERAL(CharType, "");
    static_assert(kEmptySep.empty());
    static constexpr const CharType* kNonEmptySep = STR_LITERAL(CharType, " sep ");
    static_assert(!std::basic_string_view{kNonEmptySep}.empty());

    static void test_empty_collection() {
        const std::vector<std::basic_string_view<CharType>> empty_vec{};
        assert(misc::JoinStringsCollection(kCharSep, empty_vec).empty());
        assert(misc::JoinStringsCollection(kEmptySep, empty_vec).empty());
        assert(misc::JoinStringsCollection(empty_vec).empty());
        assert(misc::JoinStringsCollection(kNonEmptySep, empty_vec).empty());
    }

    static void test_1_element_vec() {
        const std::vector<std::basic_string_view<CharType>> vec_1_elem{
            STR_LITERAL(CharType, "abcdefghijklmnopqrstuvwxyz"),
        };
        assert(misc::JoinStringsCollection(kCharSep, vec_1_elem) == vec_1_elem.front());
        assert(misc::JoinStringsCollection(kEmptySep, vec_1_elem) == vec_1_elem.front());
        assert(misc::JoinStringsCollection(vec_1_elem) == vec_1_elem.front());
        assert(misc::JoinStringsCollection(kNonEmptySep, vec_1_elem) == vec_1_elem.front());
    }

    static void test_1_element_set() {
        const std::set<std::basic_string<CharType>> set_1_elem{
            STR_LITERAL(CharType, "abcdefghijklmnopqrstuvwxyz"),
        };
        assert(misc::JoinStringsCollection(kCharSep, set_1_elem) == *set_1_elem.begin());
        assert(misc::JoinStringsCollection(kEmptySep, set_1_elem) == *set_1_elem.begin());
        assert(misc::JoinStringsCollection(set_1_elem) == *set_1_elem.begin());
        assert(misc::JoinStringsCollection(std::basic_string<CharType>{kNonEmptySep}, set_1_elem) ==
               *set_1_elem.begin());
    }

    static void test_3_elements_arr() {
        const std::array<std::basic_string<CharType>, 3> arr_3_elems{
            STR_LITERAL(CharType, "abc"),
            STR_LITERAL(CharType, "def"),
            STR_LITERAL(CharType, "ghi"),
        };
        assert(misc::JoinStringsCollection(kCharSep, arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} + kCharSep +
                   std::basic_string<CharType>{arr_3_elems[1]} + kCharSep +
                   std::basic_string<CharType>{arr_3_elems[2]});
        assert(
            misc::JoinStringsCollection(std::basic_string_view<CharType>{kEmptySep}, arr_3_elems) ==
            std::basic_string<CharType>{arr_3_elems[0]} + std::basic_string<CharType>{kEmptySep} +
                std::basic_string<CharType>{arr_3_elems[1]} +
                std::basic_string<CharType>{kEmptySep} +
                std::basic_string<CharType>{arr_3_elems[2]});
        assert(misc::JoinStringsCollection(arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} +
                   std::basic_string<CharType>{arr_3_elems[1]} +
                   std::basic_string<CharType>{arr_3_elems[2]});
        assert(misc::JoinStringsCollection(kNonEmptySep, arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} +
                   std::basic_string<CharType>{kNonEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[1]} +
                   std::basic_string<CharType>{kNonEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[2]});
    }

    static void test_list_of_empty_strings() {
        const std::list<std::basic_string_view<CharType>> list_with_empty_strings{
            kEmptySep, kEmptySep, kEmptySep, kEmptySep, kEmptySep,
        };
        assert(misc::JoinStringsCollection(kEmptySep, list_with_empty_strings).empty());
    }
};

void test() {
    JoinStringsCollectionTestSuit<char>::run();
    JoinStringsCollectionTestSuit<wchar_t>::run();
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    JoinStringsCollectionTestSuit<char8_t>::run();
#endif
    JoinStringsCollectionTestSuit<char16_t>::run();
    JoinStringsCollectionTestSuit<char32_t>::run();
}

}  // namespace join_strings_collection_test

void test_join_strings_collection() {
    join_strings_collection_test::test();
}

#endif

}  // namespace

int main() {
    test_join_strings();
#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION
    test_join_strings_collection();
#endif
}
