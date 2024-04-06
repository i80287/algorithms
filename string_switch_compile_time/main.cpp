#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <compare>
#include "StringSwitch.hpp"

static void StringSwitchExample() {
    // Fixed length array and C-style input are
    //  used for easy inspection of the asm file
    //  generated for this file.
    std::array<char, 16> input;
    scanf("%15s", input.data());
    std::string_view ans;
    static constexpr auto sw =
        StringSwitch<"abc", "def", "ghij", "foo", "bar", "baz", "abacaba",
                     "ring", "ideal", "GLn(F)">();

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
        case sw.kDefaultSwitch:
        default:
            ans = "not in the switch!";
            break;
    }
    puts(ans.data());

    static_assert(sw("abc") == 0);
    static_assert(sw("def") == 1);
    static_assert(sw("ghij") == 2);
    static_assert(sw("foo") == 3);
    static_assert(sw("bar") == 4);
    static_assert(sw("baz") == 5);
    static_assert(sw("abacaba") == 6);
    static_assert(sw("ring") == 7);
    static_assert(sw("ideal") == 8);
    static_assert(sw("GLn(F)") == 9);
    static_assert(sw.kDefaultSwitch == (sw("GLn(F)") + 1));
    static_assert(sw.kDefaultSwitch == 10);
    static_assert(sw("") == sw.kDefaultSwitch);
    static_assert(sw("a") == sw.kDefaultSwitch);
    static_assert(sw("A") == sw.kDefaultSwitch);
    static_assert(sw("de") == sw.kDefaultSwitch);
    static_assert(sw("ghi") == sw.kDefaultSwitch);
    static_assert(sw("not_in_switch") == sw.kDefaultSwitch);
}

static void ComileTimeStringMapExample() {
    static constexpr auto sw =
        StringSwitch<"cmd1", "cmd2", "cmd3", "cmd4">();
    static_assert(sw.kDefaultSwitch == sw("cmd4") + 1);
    static_assert(sw.kDefaultSwitch == 4);
    constexpr const char* map[] = {
        [sw("cmd1")]        = "called cmd1",
        [sw("cmd2")]        = "called cmd2",
        [sw("cmd3")]        = "called cmd3",
        [sw("cmd4")]        = "called cmd4",
        [sw.kDefaultSwitch] = "called something else",
    };

    std::string input;
    std::cin >> input;
    puts(map[sw(input)]);
}

int main() {
    StringSwitchExample();
    ComileTimeStringMapExample();
}
