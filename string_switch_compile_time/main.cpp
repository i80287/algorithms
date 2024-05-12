#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include "StringMatch.hpp"

static void StringMatchExample() {
    // Fixed length array and C-style input are
    //  used for easy inspection of the asm file
    //  generated for this file.
    std::array<char, 16> input;
    scanf("%15s", input.data());
    std::string_view ans;
    static constexpr auto sw = StringMatch<"abc", "def", "ghij", "foo", "bar", "baz", "qux",
                                           "abacaba", "ring", "ideal", "GLn(F)">();
    static_assert(sw("abc") == 0);
    static_assert(sw("def") == 1);
    static_assert(sw("ghij") == 2);
    static_assert(sw("foo") == 3);
    static_assert(sw("bar") == 4);
    static_assert(sw("baz") == 5);
    static_assert(sw("qux") == 6);
    static_assert(sw("abacaba") == 7);
    static_assert(sw("ring") == 8);
    static_assert(sw("ideal") == 9);
    static_assert(sw("GLn(F)") == 10);
    static_assert(sw.kDefaultValue == sw("GLn(F)") + 1);
    static_assert(sw.kDefaultValue == 11);
    static_assert(sw("") == sw.kDefaultValue);
    static_assert(sw("a") == sw.kDefaultValue);
    static_assert(sw("A") == sw.kDefaultValue);
    static_assert(sw("de") == sw.kDefaultValue);
    static_assert(sw("ghi") == sw.kDefaultValue);
    static_assert(sw("not_in_set") == sw.kDefaultValue);

    switch (sw(input.data())) {
        case sw("abc"):
            ans = "found abc";
            break;
        case sw("def"):
            ans = "found def";
            break;
        case sw("ghij"):
            ans = "found ghij";
            break;
        case sw("foo"):
            ans = "found foo";
            break;
        case sw("bar"):
            ans = "found bar";
            break;
        case sw("baz"):
            ans = "found baz";
            break;
        case sw("qux"):
            ans = "found qux";
            break;
        case sw("abacaba"):
            ans = "found abacaba";
            break;
        case sw("ring"):
            ans = "found ring";
            break;
        case sw("ideal"):
            ans = "found ideal";
            break;
        case sw("GLn(F)"):
            ans = "found GLn(F)";
            break;
        case sw.kDefaultValue:
        default:
            ans = "not in the switch!";
            break;
    }
    puts(ans.data());
}

static void ComileTimeStringMapExample() {
    static constexpr auto match = StringMatch<"text1", "text2", "text3", "text4">();
    static_assert(match("text1") == 0);
    static_assert(match("text2") == 1);
    static_assert(match("text3") == 2);
    static_assert(match("text4") == 3);
    static_assert(match("not in") == match.kDefaultValue);
    static_assert(match.kDefaultValue == 4);

    enum class SomeEnum {
        kText1,
        kText2,
        kText3,
        kText4,
        kNone,
    };

    static constexpr auto map = StringMap<
        StringsList<"text1", "text2", "text3", "text4", "Text1", "Text3">,
        TypedValuesList<SomeEnum, SomeEnum::kText1, SomeEnum::kText2, SomeEnum::kText3,
                        SomeEnum::kText4, SomeEnum::kText1, SomeEnum::kText3>,
        SomeEnum::kNone>();

    static_assert(map("text1") == SomeEnum::kText1);
    static_assert(map("text2") == SomeEnum::kText2);
    static_assert(map("text3") == SomeEnum::kText3);
    static_assert(map("text4") == SomeEnum::kText4);
    static_assert(map("Text1") == SomeEnum::kText1);
    static_assert(map("Text3") == SomeEnum::kText3);
    static_assert(map("something else") == SomeEnum::kNone);
    static_assert(map.kDefaultValue == SomeEnum::kNone);
}

int main() {
    StringMatchExample();
    ComileTimeStringMapExample();
}
