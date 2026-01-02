// clang-format off
#include "../join_strings.hpp"
// clang-format on

#include <array>
#include <cstdint>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "../../number_theory/integers_128_bit.hpp"
#include "test_strings_helper.hpp"

// clang-format off
#include "../join_strings.hpp"
// clang-format on

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
        assert(misc::join_strings(STR_LITERAL(CharType, "ab"), int8_t{1},
                                  STR_LITERAL(CharType, "cde"), 2, STR_LITERAL(CharType, "fghi"),
                                  uint16_t{3}, STR_LITERAL(CharType, "jklmn")) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));

        assert(misc::join_strings(s1, int8_t{1}, STR_LITERAL(CharType, ""), s2, 2,
                                  STR_LITERAL(CharType, ""), s3, STR_LITERAL(CharType, ""),
                                  uint16_t{3}, s4) == STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::join_strings(s2, int8_t{1}, STR_LITERAL(CharType, ""), s3, 2,
                                  STR_LITERAL(CharType, ""), s4, STR_LITERAL(CharType, ""),
                                  uint16_t{3}, s1) == STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::join_strings(s3, int8_t{1}, STR_LITERAL(CharType, ""), s4, 2,
                                  STR_LITERAL(CharType, ""), s1, STR_LITERAL(CharType, ""),
                                  uint16_t{3}, s2) == STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::join_strings(s4, int8_t{1}, STR_LITERAL(CharType, ""), s1, 2,
                                  STR_LITERAL(CharType, ""), s2, STR_LITERAL(CharType, ""),
                                  uint16_t{3}, s3) == STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::join_strings(0ULL, s1, STR_LITERAL(CharType, ""), int8_t{1}, s2,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s3,
                                  uint16_t{3}, s4) == STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::join_strings(0ULL, s2, STR_LITERAL(CharType, ""), int8_t{1}, s3,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s4,
                                  uint16_t{3}, s1) == STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::join_strings(0ULL, s3, STR_LITERAL(CharType, ""), int8_t{1}, s4,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s1,
                                  uint16_t{3}, s2) == STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::join_strings(0ULL, s4, STR_LITERAL(CharType, ""), int8_t{1}, s1,
                                  STR_LITERAL(CharType, ""), 2, STR_LITERAL(CharType, ""), s2,
                                  uint16_t{3}, s3) == STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));

        assert(misc::join_strings(s1, int8_t{1}, s2, 2, s3, uint16_t{3}, s4) ==
               STR_LITERAL(CharType, "ab1cde2fghi3jklmn"));
        assert(misc::join_strings(s2, int8_t{1}, s3, 2, s4, uint16_t{3}, s1) ==
               STR_LITERAL(CharType, "cde1fghi2jklmn3ab"));
        assert(misc::join_strings(s3, int8_t{1}, s4, 2, s1, uint16_t{3}, s2) ==
               STR_LITERAL(CharType, "fghi1jklmn2ab3cde"));
        assert(misc::join_strings(s4, int8_t{1}, s1, 2, s2, uint16_t{3}, s3) ==
               STR_LITERAL(CharType, "jklmn1ab2cde3fghi"));

        assert(misc::join_strings(0ULL, s1, int8_t{1}, s2, 2, s3, uint16_t{3}, s4) ==
               STR_LITERAL(CharType, "0ab1cde2fghi3jklmn"));
        assert(misc::join_strings(0ULL, s2, int8_t{1}, s3, 2, s4, uint16_t{3}, s1) ==
               STR_LITERAL(CharType, "0cde1fghi2jklmn3ab"));
        assert(misc::join_strings(0ULL, s3, int8_t{1}, s4, 2, s1, uint16_t{3}, s2) ==
               STR_LITERAL(CharType, "0fghi1jklmn2ab3cde"));
        assert(misc::join_strings(0ULL, s4, int8_t{1}, s1, 2, s2, uint16_t{3}, s3) ==
               STR_LITERAL(CharType, "0jklmn1ab2cde3fghi"));
    }

    static void test_without_chars() {
        test_tools::log_tests_started();

        assert(misc::join_strings<CharType>(uint8_t{1}) == STR_LITERAL(CharType, "1"));
        assert(misc::join_strings<CharType>(uint8_t{1}, 2UL) == STR_LITERAL(CharType, "12"));
        assert(misc::join_strings<CharType>(uint8_t{1}, 2UL, 3L) == STR_LITERAL(CharType, "123"));
        assert(misc::join_strings<CharType>(uint8_t{1}, 2UL, 3L, 4) ==
               STR_LITERAL(CharType, "1234"));
        assert(misc::join_strings<CharType>(uint8_t{1}, 2UL, 3L, 4, 5ULL) ==
               STR_LITERAL(CharType, "12345"));
        assert(misc::join_strings<CharType>(uint8_t{1}, static_cast<const void*>(nullptr), 2UL, 3L,
                                            nullptr, 4,
                                            5ULL) == STR_LITERAL(CharType, "1null23null45"));
    }

    static void test_with_filesystem_path() {

        assert(misc::join_strings(STR_LITERAL(CharType, "path "),
                                  std::filesystem::path{"/dev/null"},
                                  STR_LITERAL(CharType, " may exist")) ==
               STR_LITERAL(CharType, "path /dev/null may exist"));

        assert(misc::join_strings(STR_LITERAL(CharType, "path "),
                                  std::filesystem::path{"C:/Windows"},
                                  STR_LITERAL(CharType, " may exist")) ==
               STR_LITERAL(CharType, "path C:/Windows may exist"));
    }
};

void test_basic_joins() {
    JoinStringsTestSuite<char>::run();
    JoinStringsTestSuite<wchar_t>::run();
    JoinStringsTestSuite<char8_t>::run();
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

    assert(misc::join_strings<char8_t>(some::SomeEnum1{}) == to_basic_string<char8_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char8_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char8_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char8_t>(some::SomeEnum1::kSomeValue2));

    assert(misc::join_strings<char16_t>(some::SomeEnum1{}) == to_basic_string<char16_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char16_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char16_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char16_t>(some::SomeEnum1::kSomeValue2));

    assert(misc::join_strings<char32_t>(some::SomeEnum1{}) == to_basic_string<char32_t>(some::SomeEnum1{}));
    assert(misc::join_strings<char32_t>(some::SomeEnum1::kSomeValue1) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue1));
    assert(misc::join_strings<char32_t>(some::SomeEnum1::kSomeValue2) == to_basic_string<char32_t>(some::SomeEnum1::kSomeValue2));

    // clang-format on
}

void test_enums() {
    assert(misc::join_strings(E1::kValue1) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::join_strings<wchar_t>(E1::kValue1) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue1)}));
    assert(misc::join_strings(E1::kValue2) ==
           std::to_string(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));
    assert(misc::join_strings<wchar_t>(E1::kValue2) ==
           std::to_wstring(unsigned{static_cast<std::underlying_type_t<E1>>(E1::kValue2)}));

    test_custom_enum_to_string();
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

class OStringStreamWriteable final {
public:
    explicit constexpr OStringStreamWriteable(const int value) noexcept : value_{value} {}

    // [[nodiscard]] is added in order to test that ignoring
    //  returned value of the operator<< doesn't cause any warning
    //  in the join_strings()
    template <class T>
    [[nodiscard]]
    friend std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os ATTRIBUTE_LIFETIME_BOUND,
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

#ifndef __MINGW32__

template <class CharType>
class Int128TestSuite final {
public:
    static void run() {
        constexpr int128_t kNum = int128_t{551416085849} * 1'000'000'000 + 893361159;
        assert(misc::join_strings<CharType>(-kNum, 9999999999999999999ull,
                                            static_cast<uint128_t>(kNum)) ==
               STR_LITERAL(CharType,
                           "-5514160858498933611599999999999999999999551416085849893361159"));
    }
};

void test_int128() {
    Int128TestSuite<char>::run();
    Int128TestSuite<wchar_t>::run();
    Int128TestSuite<char8_t>::run();
    Int128TestSuite<char16_t>::run();
    Int128TestSuite<char32_t>::run();
}

#endif

}  // namespace join_strings_test

void test_join_strings() {
    join_strings_test::test_basic_joins();
    join_strings_test::test_enums();
    join_strings_test::test_pointers();
    join_strings_test::test_custom_ostringstream();
#ifndef __MINGW32__
    join_strings_test::test_int128();
#endif
}

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
    JoinStringsCollectionTestSuit<char8_t>::run();
    JoinStringsCollectionTestSuit<char16_t>::run();
    JoinStringsCollectionTestSuit<char32_t>::run();
}

#define W_TO_STRING_RETURN "AbCdEfGhIjKlMnOpQrStUvWxYz~!@#$%^*()_+"

namespace dummy {
class W {};

[[nodiscard]] std::string to_string(const W&) {
    return W_TO_STRING_RETURN;
}

}  // namespace dummy

#define X_TO_STRING_RETURN "0123456789ABCDEFghijklmnopqrstuvwxyz"

class X {
public:
    [[nodiscard]] std::string to_string() const {
        return X_TO_STRING_RETURN;
    }
};

#define Y_OSTREAM_REPRESENTATION "abcdefGHIJKLMNOPQRSTUVWXYZ0123456789"

class Y {
public:
    friend std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const Y&) {
        return out << Y_OSTREAM_REPRESENTATION;
    }
};

template <class CharType>
class StringConversionsTestSuite final {
public:
    static void run() {
        test_conversions();
        test_conversions_with_to_string();
    }

private:
    static void test_conversions() {
        enum class E {
            kTen = 10,
        };

        const std::basic_string<CharType> res = misc::join_strings<CharType>(
            int8_t{0}, uint8_t{1}, int16_t{2}, uint16_t{3}, int32_t{4}, uint32_t{5}, int64_t{6},
            uint64_t{7}, 8, 9, nullptr, E::kTen, static_cast<const void*>(nullptr));
        assert(res == STR_LITERAL(CharType, "0123456789null10null"));
    }

    static void test_conversions_with_to_string() {
        const auto res = misc::join_strings<CharType>(0, dummy::W{}, X{}, Y{}, nullptr);
        assert(res == STR_LITERAL(CharType,
                                  "0" W_TO_STRING_RETURN X_TO_STRING_RETURN Y_OSTREAM_REPRESENTATION
                                  "null"));
    }
};

void test_conversions() {
    StringConversionsTestSuite<char>::run();
    StringConversionsTestSuite<wchar_t>::run();
    StringConversionsTestSuite<char8_t>::run();
    StringConversionsTestSuite<char16_t>::run();
    StringConversionsTestSuite<char32_t>::run();
}

}  // namespace

int main() {
    test_join_strings();
    test_join_strings_collection();
    test_conversions();
}
