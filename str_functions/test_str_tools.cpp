#include <cassert>

#include "str_tools.hpp"

namespace {
void test_is_permutation_of() {
#if CONFIG_HAS_AT_LEAST_CXX_20
    static_assert(str_tools::is_permutation_of("", ""));
    static_assert(str_tools::is_permutation_of("ab", "ba"));
    static_assert(str_tools::is_permutation_of("ba", "ab"));
    static_assert(str_tools::is_permutation_of("aa", "aa"));
#endif

    assert(str_tools::is_permutation_of("", ""));
    assert(str_tools::is_permutation_of("ab", "ba"));
    assert(str_tools::is_permutation_of("ba", "ab"));
    assert(str_tools::is_permutation_of("aa", "aa"));

#if CONFIG_HAS_AT_LEAST_CXX_20
    static_assert(!str_tools::is_permutation_of("aa", "a"));
    static_assert(!str_tools::is_permutation_of("a", "aa"));
    static_assert(!str_tools::is_permutation_of("aa", "bb"));
#endif

    assert(!str_tools::is_permutation_of("aa", "a"));
    assert(!str_tools::is_permutation_of("a", "aa"));
    assert(!str_tools::is_permutation_of("aa", "bb"));
}

void test_unique_chars_count() {
    static_assert(str_tools::unique_chars_count("") == 0);
    static_assert(str_tools::unique_chars_count("a") == 1);
    static_assert(str_tools::unique_chars_count("aa") == 1);
    static_assert(str_tools::unique_chars_count("aaaa") == 1);
    static_assert(str_tools::unique_chars_count("aaaab") == 2);
    static_assert(str_tools::unique_chars_count("abaaab") == 2);
    static_assert(str_tools::unique_chars_count("ababaab") == 2);
    static_assert(str_tools::unique_chars_count("abdabaabc") == 4);

    assert(str_tools::unique_chars_count("") == 0);
    assert(str_tools::unique_chars_count("a") == 1);
    assert(str_tools::unique_chars_count("aa") == 1);
    assert(str_tools::unique_chars_count("aaaa") == 1);
    assert(str_tools::unique_chars_count("aaaab") == 2);
    assert(str_tools::unique_chars_count("abaaab") == 2);
    assert(str_tools::unique_chars_count("ababaab") == 2);
    assert(str_tools::unique_chars_count("abdabaabc") == 4);
}

void test_sorted_unique_chars_of() {
    assert(str_tools::sorted_unique_chars_of("") == "");
    assert(str_tools::sorted_unique_chars_of("a") == "a");
    assert(str_tools::sorted_unique_chars_of("b") == "b");
    assert(str_tools::sorted_unique_chars_of("ab") == "ab");
    assert(str_tools::sorted_unique_chars_of("ba") == "ab");
    assert(str_tools::sorted_unique_chars_of("babababababababababababababa") == "ab");
    assert(str_tools::sorted_unique_chars_of("bgdaahsjc") == "abcdghjs");
}

}  // namespace

int main() {
    test_is_permutation_of();
    test_unique_chars_count();
    test_sorted_unique_chars_of();
}
