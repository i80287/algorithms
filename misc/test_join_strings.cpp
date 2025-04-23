// clang-format off
#include "join_strings.hpp"
// clang-format on

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <vector>

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

namespace {

namespace join_strings_test {

template <class CharType>
class JoinStringsTestSuite final {
public:
    static void run() {
        test_empty();
        test_misc();
        test_without_chars();
        test_with_filesystem_path();
    }

private:
    static void test_empty() {
        test_tools::log_tests_started();

        assert(misc::join_strings(STR_LITERAL(CharType, "")) == STR_LITERAL(CharType, ""));
    }

    static void test_misc() {
        test_tools::log_tests_started();

        const std::basic_string s1 = STR_LITERAL(CharType, "ab");
        const std::basic_string_view s2 = STR_LITERAL(CharType, "cde");
        const auto s3 = STR_LITERAL(CharType, "fghi");
        const auto* const s4 = STR_LITERAL(CharType, "jklmn");

        assert(misc::join_strings(s1) == s1);
        assert(misc::join_strings(s2) == s2);
        assert(misc::join_strings(s3) == s3);
        assert(misc::join_strings(s4) == s4);

        assert(misc::join_strings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde")) ==
               STR_LITERAL(CharType, "abcde"));
        assert(misc::join_strings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde"),
                                  STR_LITERAL(CharType, "fghi")) ==
               STR_LITERAL(CharType, "abcdefghi"));
        assert(misc::join_strings(STR_LITERAL(CharType, "ab"), STR_LITERAL(CharType, "cde"),
                                  STR_LITERAL(CharType, "fghi"), STR_LITERAL(CharType, "jklmn")) ==
               STR_LITERAL(CharType, "abcdefghijklmn"));
        assert(misc::join_strings(STR_LITERAL(CharType, "ab"), 1, STR_LITERAL(CharType, "cde"), 2,
                                  STR_LITERAL(CharType, "fghi"), 3,
                                  STR_LITERAL(CharType, "jklmn")) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));

        assert(misc::join_strings(s1, 1, STR_LITERAL(CharType, ""), s2, 2,
                                  STR_LITERAL(CharType, ""), s3, STR_LITERAL(CharType, ""), 3,
                                  s4) == STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::join_strings(s2, 1, STR_LITERAL(CharType, ""), s3, 2,
                                  STR_LITERAL(CharType, ""), s4, STR_LITERAL(CharType, ""), 3,
                                  s1) == STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::join_strings(s3, 1, STR_LITERAL(CharType, ""), s4, 2,
                                  STR_LITERAL(CharType, ""), s1, STR_LITERAL(CharType, ""), 3,
                                  s2) == STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::join_strings(s4, 1, STR_LITERAL(CharType, ""), s1, 2,
                                  STR_LITERAL(CharType, ""), s2, STR_LITERAL(CharType, ""), 3,
                                  s3) == STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::join_strings(0, s1, STR_LITERAL(CharType, ""), 1, s2,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s3, 3,
                                  s4) == STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::join_strings(0, s2, STR_LITERAL(CharType, ""), 1, s3,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s4, 3,
                                  s1) == STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::join_strings(0, s3, STR_LITERAL(CharType, ""), 1, s4,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s1, 3,
                                  s2) == STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::join_strings(0, s4, STR_LITERAL(CharType, ""), 1, s1,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s2, 3,
                                  s3) == STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));

        assert(misc::join_strings(s1, 1, s2, 2, s3, 3, s4) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::join_strings(s2, 1, s3, 2, s4, 3, s1) ==
               STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::join_strings(s3, 1, s4, 2, s1, 3, s2) ==
               STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::join_strings(s4, 1, s1, 2, s2, 3, s3) ==
               STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::join_strings(0, s1, 1, s2, 2, s3, 3, s4) ==
               STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::join_strings(0, s2, 1, s3, 2, s4, 3, s1) ==
               STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::join_strings(0, s3, 1, s4, 2, s1, 3, s2) ==
               STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::join_strings(0, s4, 1, s1, 2, s2, 3, s3) ==
               STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));
    }

    static void test_without_chars() {
        test_tools::log_tests_started();

        assert(misc::join_strings<CharType>(1) == STR_LITERAL(CharType, "1"));
        assert(misc::join_strings<CharType>(1, 2) == STR_LITERAL(CharType, "12"));
        assert(misc::join_strings<CharType>(1, 2, 3) == STR_LITERAL(CharType, "123"));
        assert(misc::join_strings<CharType>(1, 2, 3, 4) == STR_LITERAL(CharType, "1234"));
        assert(misc::join_strings<CharType>(1, 2, 3, 4, 5) == STR_LITERAL(CharType, "12345"));
        assert(misc::join_strings<CharType>(1, static_cast<const void*>(nullptr), 2, 3, nullptr, 4,
                                            5) == STR_LITERAL(CharType, "1null23null45"));
    }

    static void test_with_filesystem_path() {
#ifdef JOIN_STRINGS_SUPPORTS_FILESYSTEM_PATH

        assert(misc::join_strings(STR_LITERAL(CharType, "path "),
                                  std::filesystem::path{"/dev/null"},
                                  STR_LITERAL(CharType, " may exist")) ==
               STR_LITERAL(CharType, "path /dev/null may exist"));

        assert(misc::join_strings(STR_LITERAL(CharType, "path "),
                                  std::filesystem::path{"C:/Windows"},
                                  STR_LITERAL(CharType, " may exist")) ==
               STR_LITERAL(CharType, "path C:/Windows may exist"));

#endif
    }
};

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
    kNo = false,
    kYes = true,
};

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_TO_STRING

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
            return STR_LITERAL(CharType, "kSomeValue1");
        }
        case SomeEnum1::kSomeValue2: {
            return STR_LITERAL(CharType, "kSomeValue2");
        }
        default: {
            return STR_LITERAL(CharType, "Unknown SomeEnum1 value");
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

    assert(misc::join_strings(some::SomeEnum2{}) == to_string(some::SomeEnum2{}));
    assert(misc::join_strings(some::SomeEnum2::kSomeValue1) == to_string(some::SomeEnum2::kSomeValue1));
    assert(misc::join_strings(some::SomeEnum2::kSomeValue2) == to_string(some::SomeEnum2::kSomeValue2));
    assert(misc::join_strings(some::SomeEnum2::kSomeValue3) == to_string(some::SomeEnum2::kSomeValue3));


    assert(misc::join_strings(some::SomeEnum1{}) == to_basic_string(some::SomeEnum1{}));
    assert(misc::join_strings(some::SomeEnum1::kSomeValue1) == to_basic_string(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings(some::SomeEnum1::kSomeValue2) == to_basic_string(some::SomeEnum1::kSomeValue2));

    assert(misc::join_strings<wchar_t>(some::SomeEnum1{}) == to_basic_string<wchar_t>(some::SomeEnum1{}));
    assert(misc::join_strings<wchar_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<wchar_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<wchar_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<wchar_t>(some::SomeEnum1::kSomeValue2));

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    assert(misc::join_strings<char8_t>(some::SomeEnum1{}) == to_basic_string<char8_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char8_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char8_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue2));
#endif

    assert(misc::join_strings<char16_t>(some::SomeEnum1{}) == to_basic_string<char16_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char16_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char16_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue2));

    assert(misc::join_strings<char32_t>(some::SomeEnum1{}) == to_basic_string<char32_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char32_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char32_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue2));

    // clang-format on
}

#endif

void test_enums() {
    assert(misc::join_strings(E1::kValue1) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::join_strings<wchar_t>(E1::kValue1) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::join_strings(E1::kValue2) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));
    assert(misc::join_strings<wchar_t>(E1::kValue2) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_TO_STRING
    test_custom_enum_to_string();
#endif
}

void test_pointers() {
    // clang-format off

    struct S {
        static void static_method() {}
        static void noexcept_static_method() noexcept {}
    };

    assert(misc::join_strings(nullptr) == "null");
    assert(misc::join_strings(std::nullptr_t{}) == "null");

    const S s{};

    assert(misc::join_strings(static_cast<const void*>(0)) == "null");
    assert(misc::join_strings(static_cast<const void*>(nullptr)) == "null");
    assert(misc::join_strings(static_cast<const void*>(&s)) == std::to_string(reinterpret_cast<uintptr_t>(&s)));
    assert(misc::join_strings(&s) == std::to_string(reinterpret_cast<uintptr_t>(&s)));
    assert(misc::join_strings(&test_basic_joins) == std::to_string(reinterpret_cast<uintptr_t>(&test_basic_joins)));
    assert(misc::join_strings(&test_enums) == std::to_string(reinterpret_cast<uintptr_t>(&test_enums)));
    assert(misc::join_strings(&test_pointers) == std::to_string(reinterpret_cast<uintptr_t>(&test_pointers)));

    assert(misc::join_strings(&S::static_method) == std::to_string(reinterpret_cast<uintptr_t>(&S::static_method)));
    assert(misc::join_strings(&S::noexcept_static_method) == std::to_string(reinterpret_cast<uintptr_t>(&S::noexcept_static_method)));

    // clang-format on
}

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM

class OStringStreamWriteable final {
public:
    explicit constexpr OStringStreamWriteable(const int value) noexcept : value_{value} {}

    // [[nodiscard]] is added in order to test that ignoring
    //  returned value of the operator<< doesn't cause any warning
    //  in the join_strings()
    template <class T>
    [[nodiscard]]
    friend std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os,
                                             const OStringStreamWriteable& arg) {
        return os << arg.value_;
    }

private:
    int value_;
};

template <class CharType>
class OStringStreamWritingTestSuite final {
public:
    static void run() {
        for (const int i :
             {std::numeric_limits<int>::min(), -1, 0, 1, std::numeric_limits<int>::max()}) {
            const std::string str = std::to_string(i);
            assert(misc::join_strings<CharType>(OStringStreamWriteable{i}) ==
                   std::basic_string<CharType>(str.begin(), str.end()));
        }
    }
};

void test_custom_ostringstream() {
    OStringStreamWritingTestSuite<char>::run();
}

#endif

}  // namespace join_strings_test

void test_join_strings() {
    join_strings_test::test_basic_joins();
    join_strings_test::test_enums();
    join_strings_test::test_pointers();
#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM
    join_strings_test::test_custom_ostringstream();
#endif
}

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

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
    static constexpr CharType kCharSep = STR_LITERAL(CharType, '~');
    static constexpr std::basic_string_view<CharType> kEmptySep = STR_LITERAL(CharType, "");
    static_assert(kEmptySep.empty());
    static constexpr const CharType* kNonEmptySep = STR_LITERAL(CharType, " sep ");
    static_assert(!std::basic_string_view<CharType>{kNonEmptySep}.empty(), "");

    static void test_empty_collection() {
        test_tools::log_tests_started();

        const std::vector<std::basic_string_view<CharType>> empty_vec{};
        assert(misc::join_strings_collection(kCharSep, empty_vec).empty());
        assert(misc::join_strings_collection(kEmptySep, empty_vec).empty());
        assert(misc::join_strings_collection(empty_vec).empty());
        assert(misc::join_strings_collection(kNonEmptySep, empty_vec).empty());
    }

    static void test_1_element_vec() {
        test_tools::log_tests_started();

        const std::vector<std::basic_string_view<CharType>> vec_1_elem{
            STR_LITERAL(CharType, "abcdefghijklmnopqrstuvwxyz"),
        };
        assert(misc::join_strings_collection(kCharSep, vec_1_elem) == vec_1_elem.front());
        assert(misc::join_strings_collection(kEmptySep, vec_1_elem) == vec_1_elem.front());
        assert(misc::join_strings_collection(vec_1_elem) == vec_1_elem.front());
        assert(misc::join_strings_collection(kNonEmptySep, vec_1_elem) == vec_1_elem.front());
    }

    static void test_1_element_set() {
        test_tools::log_tests_started();

        const std::set<std::basic_string<CharType>> set_1_elem{
            STR_LITERAL(CharType, "abcdefghijklmnopqrstuvwxyz"),
        };
        assert(misc::join_strings_collection(kCharSep, set_1_elem) == *set_1_elem.begin());
        assert(misc::join_strings_collection(kEmptySep, set_1_elem) == *set_1_elem.begin());
        assert(misc::join_strings_collection(set_1_elem) == *set_1_elem.begin());
        assert(misc::join_strings_collection(std::basic_string<CharType>{kNonEmptySep},
                                             set_1_elem) == *set_1_elem.begin());
    }

    static void test_3_elements_arr() {
        test_tools::log_tests_started();

        const std::array<std::basic_string<CharType>, 3> arr_3_elems{
            STR_LITERAL(CharType, "abc"),
            STR_LITERAL(CharType, "def"),
            STR_LITERAL(CharType, "ghi"),
        };

#if CONFIG_GNUC_AT_LEAST(15, 0) && !defined(__clang__)
// Bug in GCC 15+: false positive may occur with warning -Walloc-size-larger-than=x
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wno-alloc-size-larger-than"
#endif

        assert(misc::join_strings_collection(kCharSep, arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} + kCharSep +
                   std::basic_string<CharType>{arr_3_elems[1]} + kCharSep +
                   std::basic_string<CharType>{arr_3_elems[2]});
        assert(misc::join_strings_collection(std::basic_string_view<CharType>{kEmptySep},
                                             arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} +
                   std::basic_string<CharType>{kEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[1]} +
                   std::basic_string<CharType>{kEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[2]});
        assert(misc::join_strings_collection(arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} +
                   std::basic_string<CharType>{arr_3_elems[1]} +
                   std::basic_string<CharType>{arr_3_elems[2]});
        assert(misc::join_strings_collection(kNonEmptySep, arr_3_elems) ==
               std::basic_string<CharType>{arr_3_elems[0]} +
                   std::basic_string<CharType>{kNonEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[1]} +
                   std::basic_string<CharType>{kNonEmptySep} +
                   std::basic_string<CharType>{arr_3_elems[2]});

#if CONFIG_GNUC_AT_LEAST(15, 0) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
    }

    static void test_list_of_empty_strings() {
        test_tools::log_tests_started();

        const std::list<std::basic_string_view<CharType>> list_with_empty_strings{
            kEmptySep, kEmptySep, kEmptySep, kEmptySep, kEmptySep,
        };
        assert(misc::join_strings_collection(kEmptySep, list_with_empty_strings).empty());
    }
};

void test_join_strings_collection() {
    JoinStringsCollectionTestSuit<char>::run();
    JoinStringsCollectionTestSuit<wchar_t>::run();
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    JoinStringsCollectionTestSuit<char8_t>::run();
#endif
    JoinStringsCollectionTestSuit<char16_t>::run();
    JoinStringsCollectionTestSuit<char32_t>::run();
}

#endif

template <class CharType>
class IsWhiteSpaceTestSuite final {
public:
    static void run() {
        test_whitespace_chars();
        test_non_whitespace_chars();
        test_whitespace_strings();
    }

private:
    static void test_whitespace_chars() {
        test_tools::log_tests_started();

        assert(misc::is_whitespace(STR_LITERAL(CharType, ' ')));
        assert(misc::is_whitespace(STR_LITERAL(CharType, '\t')));
        assert(misc::is_whitespace(STR_LITERAL(CharType, '\v')));
        assert(misc::is_whitespace(STR_LITERAL(CharType, '\f')));
        assert(misc::is_whitespace(STR_LITERAL(CharType, '\r')));
        assert(misc::is_whitespace(STR_LITERAL(CharType, '\n')));
    }

    static void test_non_whitespace_chars() {
        test_tools::log_tests_started();

        assert(!misc::is_whitespace(STR_LITERAL(CharType, 'a')));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, 'z')));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, '0')));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, '9')));
    }

    static void test_whitespace_strings() {
        test_tools::log_tests_started();

        assert(misc::is_whitespace(STR_LITERAL(CharType, "")));
        assert(misc::is_whitespace(STR_LITERAL(CharType, "        ")));
        assert(misc::is_whitespace(STR_LITERAL(CharType, " \t\v\f\r\n")));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, " \t\v\f\r\nq")));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, " \t\v\fq\r\n")));
        assert(!misc::is_whitespace(STR_LITERAL(CharType, "q \t\v\f\r\n")));

        assert(
            misc::is_whitespace(std::basic_string<CharType>{STR_LITERAL(CharType, " \t\v\f\r\n")}));
        assert(misc::is_whitespace(
            std::basic_string_view<CharType>{STR_LITERAL(CharType, " \t\v\f\r\n")}));
    }
};

void test_is_white_space() {
    IsWhiteSpaceTestSuite<char>::run();
    IsWhiteSpaceTestSuite<wchar_t>::run();
    IsWhiteSpaceTestSuite<char16_t>::run();
    IsWhiteSpaceTestSuite<char32_t>::run();
}

template <class CharType>
class TrimTestSuite final {
public:
    static void run() {
        test_trim_empty();
        test_trim_spaces();
        test_trim_alphas();
        test_trim_digits();
        test_trim_alpha_digits();
        test_trim_hex_digits();
        test_trim_chars();
    }

private:
    static void test_trim_empty() {
        test_tools::log_tests_started();
        assert(misc::trim(STR_LITERAL(CharType, "")) == STR_LITERAL(CharType, ""));
    }

    static void test_trim_spaces() {
        test_tools::log_tests_started();

        assert(misc::trim(STR_LITERAL(CharType, ""), misc::whitespace_tag{}) ==
               STR_LITERAL(CharType, ""));

        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\n")) == STR_LITERAL(CharType, ""));
        assert(misc::trim(std::basic_string<CharType>{STR_LITERAL(CharType, " \t\v\r\n")}) ==
               STR_LITERAL(CharType, ""));
        assert(misc::trim(std::basic_string_view<CharType>{STR_LITERAL(CharType, " \t\v\r\n")}) ==
               STR_LITERAL(CharType, ""));

        assert(misc::trim(STR_LITERAL(CharType, "abc")) == STR_LITERAL(CharType, "abc"));
        assert(misc::trim(STR_LITERAL(CharType, "abc \t\v\r\n")) == STR_LITERAL(CharType, "abc"));
        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\nabc")) == STR_LITERAL(CharType, "abc"));
        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\nabc \t\v\r\n")) ==
               STR_LITERAL(CharType, "abc"));

        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\nabc \t\v\r\n"), misc::whitespace_tag{}) ==
               STR_LITERAL(CharType, "abc"));
        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\nabc \t\v\r\n"), misc::whitespace_tag{}) ==
               STR_LITERAL(CharType, "abc"));
        assert(misc::trim(STR_LITERAL(CharType, " \t\v\r\nabc \t\v\r\n"), misc::whitespace_tag{}) ==
               STR_LITERAL(CharType, "abc"));
    }

    static void test_trim_alphas() {
        test_tools::log_tests_started();

        test_trim_with_tag<misc::alpha_tag>(STR_LITERAL(CharType, ""), STR_LITERAL(CharType, ""));

        test_trim_with_tag<misc::alpha_tag>(STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23"),
                                            STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23"));

        test_trim_with_tag<misc::alpha_tag>(
            STR_LITERAL(CharType, "abcddaDWADWh17fe28D*lD$@^&hajDAw23ASdhjad"),
            STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23"));
        test_trim_with_tag<misc::alpha_tag>(STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23ASdhjad"),
                                            STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23"));
        test_trim_with_tag<misc::alpha_tag>(
            STR_LITERAL(CharType, "abcddaDWADWh17fe28D*lD$@^&hajDAw23"),
            STR_LITERAL(CharType, "17fe28D*lD$@^&hajDAw23"));
    }

    static void test_trim_digits() {
        test_tools::log_tests_started();

        test_trim_with_tag<misc::digit_tag>(STR_LITERAL(CharType, ""), STR_LITERAL(CharType, ""));

        test_trim_with_tag<misc::digit_tag>(STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$"),
                                            STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$"));

        test_trim_with_tag<misc::digit_tag>(
            STR_LITERAL(CharType, "382734AhdjwAW273*38@*34@dajwkDW$2389"),
            STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$"));
        test_trim_with_tag<misc::digit_tag>(STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$2389"),
                                            STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$"));
        test_trim_with_tag<misc::digit_tag>(
            STR_LITERAL(CharType, "382734AhdjwAW273*38@*34@dajwkDW$"),
            STR_LITERAL(CharType, "AhdjwAW273*38@*34@dajwkDW$"));
    }

    static void test_trim_alpha_digits() {
        test_tools::log_tests_started();

        test_trim_with_tag<misc::alpha_digit_tag>(STR_LITERAL(CharType, ""),
                                                  STR_LITERAL(CharType, ""));

        test_trim_with_tag<misc::alpha_digit_tag>(STR_LITERAL(CharType, "@^&#@#&$#&)($"),
                                                  STR_LITERAL(CharType, "@^&#@#&$#&)($"));

        test_trim_with_tag<misc::alpha_digit_tag>(
            STR_LITERAL(CharType, "ADhjawhdjawh27837adsjKA@^&#@#&$#&)($sjkdakdj28938192"),
            STR_LITERAL(CharType, "@^&#@#&$#&)($"));
        test_trim_with_tag<misc::alpha_digit_tag>(
            STR_LITERAL(CharType, "@^&#@#&$#&)($sjkdakdj28938192"),
            STR_LITERAL(CharType, "@^&#@#&$#&)($"));
        test_trim_with_tag<misc::alpha_digit_tag>(
            STR_LITERAL(CharType, "ADhjawhdjawh27837adsjKA@^&#@#&$#&)($"),
            STR_LITERAL(CharType, "@^&#@#&$#&)($"));
    }

    static void test_trim_hex_digits() {
        test_tools::log_tests_started();

        test_trim_with_tag<misc::hex_digit_tag>(STR_LITERAL(CharType, ""),
                                                STR_LITERAL(CharType, ""));

        test_trim_with_tag<misc::hex_digit_tag>(
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"),
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"));

        test_trim_with_tag<misc::hex_digit_tag>(
            STR_LITERAL(CharType, "2189389AcbDefGHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ49832849DfaB49349"),
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"));
        test_trim_with_tag<misc::hex_digit_tag>(
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ49832849DfaB49349"),
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"));
        test_trim_with_tag<misc::hex_digit_tag>(
            STR_LITERAL(CharType, "2189389AcbDefGHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"),
            STR_LITERAL(CharType, "GHhugeGJk@^&#@#&$#&)($zjGhjGEOpQ"));
    }

    static void test_trim_chars() {
        test_tools::log_tests_started();

        test_trim_chars_impl(STR_LITERAL(CharType, "yyyyyyyabcyyyyyy"), STR_LITERAL(CharType, "y"),
                             STR_LITERAL(CharType, "abc"));
        test_trim_chars_impl(STR_LITERAL(CharType, "xyxyxyabcdxydxyxy"),
                             STR_LITERAL(CharType, "yx"), STR_LITERAL(CharType, "abcdxyd"));
    }

    template <size_t N, size_t M, size_t K>
    static void test_trim_chars_impl(const CharType (&str)[N],
                                     const CharType (&trim_chars)[M],
                                     const CharType (&res_str)[K]) {
        assert(misc::trim(str, trim_chars) == res_str);
        assert(misc::trim(std::basic_string<CharType>{str}, trim_chars) == res_str);
        assert(misc::trim(std::basic_string_view<CharType>{str}, trim_chars) == res_str);

        assert(misc::trim(str, std::basic_string<CharType>{trim_chars}) == res_str);
        assert(misc::trim(std::basic_string<CharType>{str},
                          std::basic_string<CharType>{trim_chars}) == res_str);
        assert(misc::trim(std::basic_string_view<CharType>{str},
                          std::basic_string<CharType>{trim_chars}) == res_str);

        assert(misc::trim(str, std::basic_string_view<CharType>{trim_chars}) == res_str);
        assert(misc::trim(std::basic_string<CharType>{str},
                          std::basic_string_view<CharType>{trim_chars}) == res_str);
        assert(misc::trim(std::basic_string_view<CharType>{str},
                          std::basic_string_view<CharType>{trim_chars}) == res_str);
    }

    template <class TagType, size_t N, size_t K>
    static void test_trim_with_tag(const CharType (&str)[N], const CharType (&res_str)[K]) {
        static_assert(std::is_base_of_v<misc::trim_tag, TagType>, "");

        constexpr TagType tag{};
        assert(misc::trim(str, tag) == res_str);
        assert(misc::trim(std::basic_string<CharType>{str}, tag) == res_str);
        assert(misc::trim(std::basic_string_view<CharType>{str}, tag) == res_str);
    }
};

void test_trim_strings() {
    TrimTestSuite<char>::run();
    TrimTestSuite<wchar_t>::run();
}

template <class CharType>
class ToLowerTestSuite final {
public:
    static void run() {
        test_empty();
        test_non_empty_strings();
    }

private:
    static void test_empty() {
        test_tools::log_tests_started();

        assert(misc::to_lower(STR_LITERAL(CharType, "")) == STR_LITERAL(CharType, ""));
    }

    static void test_non_empty_strings() {
        test_tools::log_tests_started();

        assert(misc::to_lower(STR_LITERAL(CharType, "abcdef")) == STR_LITERAL(CharType, "abcdef"));
        assert(misc::to_lower(STR_LITERAL(CharType, "Abcdef")) == STR_LITERAL(CharType, "abcdef"));
        assert(misc::to_lower(STR_LITERAL(CharType, "abcdeF")) == STR_LITERAL(CharType, "abcdef"));
        assert(misc::to_lower(STR_LITERAL(CharType, " ABCDEF012345689 ")) ==
               STR_LITERAL(CharType, " abcdef012345689 "));
        assert(misc::to_lower(STR_LITERAL(CharType, " AbCdEf012345689 ")) ==
               STR_LITERAL(CharType, " abcdef012345689 "));

        assert(misc::to_lower(std::basic_string<CharType>{STR_LITERAL(CharType, "AbCdEf")}) ==
               STR_LITERAL(CharType, "abcdef"));
        assert(misc::to_lower(std::basic_string_view<CharType>{STR_LITERAL(CharType, "AbCdEf")}) ==
               STR_LITERAL(CharType, "abcdef"));
    }
};

void test_to_lower() {
    ToLowerTestSuite<char>::run();
    ToLowerTestSuite<wchar_t>::run();
}

template <class CharType>
class ToUpperTestSuite final {
public:
    static void run() {
        test_empty();
        test_non_empty_strings();
    }

private:
    static void test_empty() {
        test_tools::log_tests_started();

        assert(misc::to_upper(STR_LITERAL(CharType, "")) == STR_LITERAL(CharType, ""));
    }

    static void test_non_empty_strings() {
        test_tools::log_tests_started();

        assert(misc::to_upper(STR_LITERAL(CharType, "abcdef")) == STR_LITERAL(CharType, "ABCDEF"));
        assert(misc::to_upper(STR_LITERAL(CharType, "Abcdef")) == STR_LITERAL(CharType, "ABCDEF"));
        assert(misc::to_upper(STR_LITERAL(CharType, "abcdeF")) == STR_LITERAL(CharType, "ABCDEF"));
        assert(misc::to_upper(STR_LITERAL(CharType, " ABCDEF012345689 ")) ==
               STR_LITERAL(CharType, " ABCDEF012345689 "));
        assert(misc::to_upper(STR_LITERAL(CharType, " AbCdEf012345689 ")) ==
               STR_LITERAL(CharType, " ABCDEF012345689 "));

        assert(misc::to_upper(std::basic_string<CharType>{STR_LITERAL(CharType, "AbCdEf")}) ==
               STR_LITERAL(CharType, "ABCDEF"));
        assert(misc::to_upper(std::basic_string_view<CharType>{STR_LITERAL(CharType, "AbCdEf")}) ==
               STR_LITERAL(CharType, "ABCDEF"));
    }
};

void test_to_upper() {
    ToUpperTestSuite<char>::run();
    ToUpperTestSuite<wchar_t>::run();
}

}  // namespace

int main() {
    test_join_strings();
#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION
    test_join_strings_collection();
#endif
    test_is_white_space();
    test_trim_strings();
    test_to_lower();
    test_to_upper();
}
