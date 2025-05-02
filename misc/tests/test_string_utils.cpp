// clang-format off
#include "../string_utils.hpp"
// clang-format on

#include <string>
#include <string_view>

#include "test_strings_helper.hpp"

// clang-format off
#include "../string_utils.hpp"
// clang-format on

namespace {

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
    test_is_white_space();
    test_trim_strings();
    test_to_lower();
    test_to_upper();
}
