#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "StringSwitch.hpp"

int main() {
    char input[16];
    scanf("%15s", input);
    const char* ans;

    static constexpr auto sw =
        StringSwitch<"abc", "def", "ghij", "foo", "bar", "baz", "abacaba",
                     "ring", "ideal", "GLn(F)">();
    switch (sw.Switch(input)) {
        case 0:
            ans = "found abc";
            break;
        case 1:
            ans = "found def";
            break;
        case 2:
            ans = "found ghij";
            break;
        case 3:
            ans = "found foo";
            break;
        case 4:
            ans = "found bar";
            break;
        case 5:
            ans = "found baz";
            break;
        case 6:
            ans = "found abacaba";
            break;
        case 7:
            ans = "found ring";
            break;
        case 8:
            ans = "found ideal";
            break;
        case 9:
            ans = "found GLn(F)";
            break;
        case sw.kDefaultSwitch:
        default:
            ans = "not in the switch!";
            break;
    }
    puts(ans);

    static_assert(sw.Switch("abc") == 0);
    static_assert(sw.Switch("def") == 1);
    static_assert(sw.Switch("ghij") == 2);
    static_assert(sw.Switch("foo") == 3);
    static_assert(sw.Switch("bar") == 4);
    static_assert(sw.Switch("baz") == 5);
    static_assert(sw.Switch("abacaba") == 6);
    static_assert(sw.Switch("ring") == 7);
    static_assert(sw.Switch("ideal") == 8);
    static_assert(sw.Switch("GLn(F)") == 9);
    static_assert(sw.Switch(nullptr) == sw.kDefaultSwitch);
    static_assert(sw.Switch("") == sw.kDefaultSwitch);
    static_assert(sw.Switch("a") == sw.kDefaultSwitch);
    static_assert(sw.Switch("A") == sw.kDefaultSwitch);
    static_assert(sw.Switch("de") == sw.kDefaultSwitch);
    static_assert(sw.Switch("ghi") == sw.kDefaultSwitch);
    static_assert(sw.Switch("not_in_switch") == sw.kDefaultSwitch);
}
