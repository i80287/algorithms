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
    switch (sw.Switch(input)) {
        case sw.Case("abc"):
            ans = "found abc";
            break;
        case sw.Case("def"):
            ans = "found def";
            break;
        case sw.Case("ghij"):
            ans = "found ghij";
            break;
        case sw.Case("foo"):
            ans = "found foo";
            break;
        case sw.Case("bar"):
            ans = "found bar";
            break;
        case sw.Case("baz"):
            ans = "found baz";
            break;
        case sw.Case("abacaba"):
            ans = "found abacaba";
            break;
        case sw.Case("ring"):
            ans = "found ring";
            break;
        case sw.Case("ideal"):
            ans = "found ideal";
            break;
        case sw.Case("GLn(F)"):
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

    static_assert(sw.Case("abc") == 0);
    static_assert(sw.Case("def") == 1);
    static_assert(sw.Case("ghij") == 2);
    static_assert(sw.Case("foo") == 3);
    static_assert(sw.Case("bar") == 4);
    static_assert(sw.Case("baz") == 5);
    static_assert(sw.Case("abacaba") == 6);
    static_assert(sw.Case("ring") == 7);
    static_assert(sw.Case("ideal") == 8);
    static_assert(sw.Case("GLn(F)") == 9);
    static_assert(sw.Case("") == sw.kDefaultSwitch);
    static_assert(sw.Case("a") == sw.kDefaultSwitch);
    static_assert(sw.Case("A") == sw.kDefaultSwitch);
    static_assert(sw.Case("de") == sw.kDefaultSwitch);
    static_assert(sw.Case("ghi") == sw.kDefaultSwitch);
    static_assert(sw.Case("not_in_switch") == sw.kDefaultSwitch);
}
