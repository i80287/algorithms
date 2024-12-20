#include <cassert>
#include <string>
#include <string_view>

#include "join_strings.hpp"

int main() {
    {
        assert(misc::JoinStrings("") == "");
        assert(misc::JoinStrings("ab", "cde") == "abcde");
        assert(misc::JoinStrings("ab", "cde", "fghi") == "abcdefghi");
        assert(misc::JoinStrings("ab", "cde", "fghi", "jklmn") == "abcdefghijklmn");
        assert(misc::JoinStrings("ab", 1, "cde", 2, "fghi", 3, "jklmn") == "ab1cde2fghi3jklmn");

        std::string s1      = "ab";
        std::string_view s2 = "cde";
        const char s3[]     = "fghi";
        const char* s4      = "jklmn";

        assert(misc::JoinStrings(s1, 1, "", s2, 2, "", s3, "", 3, s4) == "ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(s2, 1, "", s3, 2, "", s4, "", 3, s1) == "cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(s3, 1, "", s4, 2, "", s1, "", 3, s2) == "fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(s4, 1, "", s1, 2, "", s2, "", 3, s3) == "jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(0, s1, "", 1, s2, "", 2, "", s3, 3, s4) == "0ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(0, s2, "", 1, s3, "", 2, "", s4, 3, s1) == "0cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(0, s3, "", 1, s4, "", 2, "", s1, 3, s2) == "0fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(0, s4, "", 1, s1, "", 2, "", s2, 3, s3) == "0jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(s1, 1, s2, 2, s3, 3, s4) == "ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(s2, 1, s3, 2, s4, 3, s1) == "cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(s3, 1, s4, 2, s1, 3, s2) == "fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(s4, 1, s1, 2, s2, 3, s3) == "jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(0, s1, 1, s2, 2, s3, 3, s4) == "0ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(0, s2, 1, s3, 2, s4, 3, s1) == "0cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(0, s3, 1, s4, 2, s1, 3, s2) == "0fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(0, s4, 1, s1, 2, s2, 3, s3) == "0jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(1) == "1");
        assert(misc::JoinStrings(1, 2) == "12");
        assert(misc::JoinStrings(1, 2, 3) == "123");
        assert(misc::JoinStrings(1, 2, 3, 4) == "1234");
        assert(misc::JoinStrings(1, 2, 3, 4, 5) == "12345");
    }

    {
        assert(misc::JoinStrings(L"") == L"");
        assert(misc::JoinStrings(L"ab", L"cde") == L"abcde");
        assert(misc::JoinStrings(L"ab", L"cde", L"fghi") == L"abcdefghi");
        assert(misc::JoinStrings(L"ab", L"cde", L"fghi", L"jklmn") == L"abcdefghijklmn");
        assert(misc::JoinStrings(L"ab", 1, L"cde", 2, L"fghi", 3, L"jklmn") ==
               L"ab1cde2fghi3jklmn");

        std::wstring s1      = L"ab";
        std::wstring_view s2 = L"cde";
        const wchar_t s3[]   = L"fghi";
        const wchar_t* s4    = L"jklmn";

        assert(misc::JoinStrings(s1, 1, s2, 2, s3, 3, s4) == L"ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(s2, 1, s3, 2, s4, 3, s1) == L"cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(s3, 1, s4, 2, s1, 3, s2) == L"fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(s4, 1, s1, 2, s2, 3, s3) == L"jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(0, s1, 1, s2, 2, s3, 3, s4) == L"0ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(0, s2, 1, s3, 2, s4, 3, s1) == L"0cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(0, s3, 1, s4, 2, s1, 3, s2) == L"0fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(0, s4, 1, s1, 2, s2, 3, s3) == L"0jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(s1, 1, L"", s2, 2, L"", s3, L"", 3, s4) == L"ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(s2, 1, L"", s3, 2, L"", s4, L"", 3, s1) == L"cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(s3, 1, L"", s4, 2, L"", s1, L"", 3, s2) == L"fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(s4, 1, L"", s1, 2, L"", s2, L"", 3, s3) == L"jklmn1ab2cde3fghi");

        assert(misc::JoinStrings(0, s1, L"", 1, s2, L"", 2, L"", s3, 3, s4) ==
               L"0ab1cde2fghi3jklmn");
        assert(misc::JoinStrings(0, s2, L"", 1, s3, L"", 2, L"", s4, 3, s1) ==
               L"0cde1fghi2jklmn3ab");
        assert(misc::JoinStrings(0, s3, L"", 1, s4, L"", 2, L"", s1, 3, s2) ==
               L"0fghi1jklmn2ab3cde");
        assert(misc::JoinStrings(0, s4, L"", 1, s1, L"", 2, L"", s2, 3, s3) ==
               L"0jklmn1ab2cde3fghi");

        assert(misc::JoinStrings<wchar_t>(1) == L"1");
        assert(misc::JoinStrings<wchar_t>(1, 2) == L"12");
        assert(misc::JoinStrings<wchar_t>(1, 2, 3) == L"123");
        assert(misc::JoinStrings<wchar_t>(1, 2, 3, 4) == L"1234");
        assert(misc::JoinStrings<wchar_t>(1, 2, 3, 4, 5) == L"12345");
    }
}
