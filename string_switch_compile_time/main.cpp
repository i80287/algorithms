#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "StringSwitch.hpp"

int main() {
    char input[16];
    scanf("%15s", input);
    const char* ans;

    static constexpr auto sw = StringSwitch<"abc", "def", "ghij", "foo", "bar", "baz",
                                            "abacaba", "ring", "ideal", "GLn(F)">();
    switch (sw(input)) {
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
    puts(ans);

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
    static_assert(sw("") == sw.kDefaultSwitch);
    static_assert(sw("a") == sw.kDefaultSwitch);
    static_assert(sw("A") == sw.kDefaultSwitch);
    static_assert(sw("de") == sw.kDefaultSwitch);
    static_assert(sw("ghi") == sw.kDefaultSwitch);
    static_assert(sw("not_in_switch") == sw.kDefaultSwitch);
}
