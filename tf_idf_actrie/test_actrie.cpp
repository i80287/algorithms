#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef NDEBUG
#error("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

#include <cassert>

#include "actrie.hpp"

namespace {

namespace actrie_tests {

using std::size_t;

template <size_t PatternsSize>
[[nodiscard]] bool test_actrie(
    const std::string_view (&patterns)[PatternsSize],
    std::string_view text,
    const std::vector<std::pair<std::string_view, size_t>>& expected_occurances) {
    actrie::ACTrieBuilder builder;
    for (std::string_view pattern : patterns) {
        if (!builder.AddPattern(pattern)) {
            return false;
        }
    }
    for (const std::string_view pattern : patterns) {
        if (!builder.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (builder.PatternsSize() != PatternsSize) {
        return false;
    }
    const actrie::ACTrie t = std::move(builder).Build();
    for (const std::string_view pattern : patterns) {
        if (!t.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (t.PatternsSize() != PatternsSize) {
        return false;
    }
    std::vector<std::pair<std::string_view, size_t>> found_occurances;
    found_occurances.reserve(expected_occurances.size());
    t.FindAllSubstringsInText(text, [&found_occurances](std::string_view found_word,
                                                        size_t start_index_in_original_text) {
        found_occurances.emplace_back(found_word, start_index_in_original_text);
    });
    return found_occurances == expected_occurances;
}

void test0() {
    constexpr std::string_view patterns[] = {
        "a", "ab", "ba", "aa", "bb", "fasb",
    };
    constexpr std::string_view text                                      = "ababcdacafaasbfasbabcc";
    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        {"a", 0},  {"ab", 0},    {"ba", 1},  {"a", 2},   {"ab", 2},
        {"a", 6},  {"a", 8},     {"a", 10},  {"aa", 10}, {"a", 11},
        {"a", 15}, {"fasb", 14}, {"ba", 17}, {"a", 18},  {"ab", 18},
    };

    assert(test_actrie(patterns, text, expected_occurances));
}

void test1() {
    constexpr std::string_view patterns[] = {
        "ABC",
        "CDE",
        "CDEF",
    };
    constexpr std::string_view text =
        "ABCDEFGHABCDEFGADCVABCDEBACBCBABDBEBCBABABBCDEBCBABDEBCABDBCBACABCDBEBACBCDEWBCBABCDE";
    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        {"ABC", 0},  {"CDE", 2},  {"CDEF", 2}, {"ABC", 8},  {"CDE", 10}, {"CDEF", 10}, {"ABC", 19},
        {"CDE", 21}, {"CDE", 43}, {"ABC", 63}, {"CDE", 73}, {"ABC", 80}, {"CDE", 82},
    };

    assert(test_actrie(patterns, text, expected_occurances));
}

void test2() {
    constexpr std::string_view patterns[] = {
        "aba", "baca", "abacaba", "ccbba", "cabaaba",
    };
    constexpr std::string_view text =
        "ababcbbacbcabaabaacbacbbacbbabcbabcbcabaabaabcabaabacabaabacbabbbacbabacbabbacbcabacabcbcb"
        "acabaababcbabbacacbbcbcababbcbcbacabcabacbcababacababcbabccaababacabcbabcbacbabcabcbbababa"
        "caababababcbbcbcbcbcbcbababcbabcabccbbcbcbcabaabacabbacbabca";
    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        {"aba", 0},      {"aba", 11},     {"cabaaba", 10},  {"aba", 14},     {"aba", 37},
        {"cabaaba", 36}, {"aba", 40},     {"aba", 46},      {"cabaaba", 45}, {"aba", 49},
        {"baca", 50},    {"abacaba", 49}, {"aba", 53},      {"cabaaba", 52}, {"aba", 56},
        {"aba", 68},     {"aba", 80},     {"baca", 81},     {"baca", 89},    {"aba", 92},
        {"cabaaba", 91}, {"aba", 95},     {"baca", 103},    {"aba", 113},    {"baca", 121},
        {"aba", 127},    {"aba", 133},    {"aba", 135},     {"baca", 136},   {"abacaba", 135},
        {"aba", 139},    {"aba", 150},    {"aba", 152},     {"baca", 153},   {"aba", 175},
        {"aba", 177},    {"baca", 178},   {"aba", 182},     {"aba", 184},    {"aba", 186},
        {"aba", 203},    {"aba", 223},    {"cabaaba", 222}, {"aba", 226},    {"baca", 227},
    };

    assert(test_actrie(patterns, text, expected_occurances));
}

}  // namespace actrie_tests

namespace replacing_actrie_tests {

template <bool IsCaseInsensetive = true, size_t PatternsSize>
[[nodiscard]] bool test_replacing_actrie(
    const std::string_view (&patterns_with_replacements)[PatternsSize][2],
    std::string& input_text,
    const std::string_view expected,
    const bool replace_all_occurances) {
    using BuilderType = actrie::ReplacingACTrieBuilder<
        /* AlphabetStart = */ '-',
        /* AlphabetEnd = */ '}',
        /* IsCaseInsensetive = */ IsCaseInsensetive>;
    auto builder = BuilderType::WithCapacity(PatternsSize);
    for (const auto [pattern, replacement] : patterns_with_replacements) {
        if (!builder.AddPatternWithReplacement(pattern, std::string(replacement))) {
            return false;
        }
    }
    for (const auto& [pattern, _] : patterns_with_replacements) {
        if (!builder.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (builder.PatternsSize() != PatternsSize) {
        return false;
    }
    const actrie::ReplacingACTrie t = std::move(builder).Build();
    for (const auto& [pattern, _] : patterns_with_replacements) {
        if (!t.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (t.PatternsSize() != PatternsSize) {
        return false;
    }

    if (replace_all_occurances) {
        t.ReplaceAtMostKOccurances(input_text);
    } else {
        t.ReplaceFirstOccurance(input_text);
    }
    return input_text == expected;
}

void test0() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"fasb", "xfasbx"},
    };
    std::string input_text     = "ababcdacafaasbfasbabcc";
    const std::string expected = "cdcdcdacafccsbxfasbxcdcc";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test1() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"xfasbx", "fasb"},
    };
    std::string input_text     = "ababcdacafaasbxfasbxabcc";
    const std::string expected = "cdcdcdacafccsbfasbcdcc";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test2() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"LM", "0000"}, {"GHI", "111111"}, {"BCD", "2222222"}, {"nop", "3333"}, {"jk", "44444"},
    };
    std::string input_text     = "ABCDEFGHIJKLMNOP";
    const std::string expected = "A2222222EF1111114444400003333";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test3() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "111111111111111111111111"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "no"},
    };
    std::string input_text     = "ABCDEFGHIJKLMNOP";
    const std::string expected = "111111111111111111111111cdefGHIjkLMnoP";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test4() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "111111111111111111111111"},
    };
    std::string input_text     = "ABCDEFGHIJKLMNOP";
    const std::string expected = "abcdefGHIjkLM111111111111111111111111P";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test5() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "111111111111111111111111"}, {"JK", "jk"}, {"NO", "no"},
    };
    std::string input_text     = "ABCDEFGHIJKLMNOP";
    const std::string expected = "abcd111111111111111111111111GHIjkLMnoP";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test6() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"kernel", "Kewnel"}, {"linux", "Linuwu"},         {"debian", "Debinyan"},
        {"ubuntu", "Uwuntu"}, {"windows", "WinyandOwOws"},
    };
    std::string input_text     = "linux kernel; debian os; ubuntu os; windows os";
    const std::string expected = "Linuwu Kewnel; Debinyan os; Uwuntu os; WinyandOwOws os";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test7() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"brew-cask", "bwew-cawsk"},
        {"brew-cellar", "bwew-cewwaw"},
        {"emerge", "emewge"},
        {"flatpak", "fwatpakkies"},
        {"pacman", "pacnyan"},
        {"port", "powt"},
        {"rpm", "rawrpm"},
        {"snap", "snyap"},
        {"zypper", "zyppew"},

        {"lenovo", "LenOwO"},
        {"cpu", "CPUwU"},
        {"core", "Cowe"},
        {"gpu", "GPUwU"},
        {"graphics", "Gwaphics"},
        {"corporation", "COwOpowation"},
        {"nvidia", "NyaVIDIA"},
        {"mobile", "Mwobile"},
        {"intel", "Inteww"},
        {"radeon", "Radenyan"},
        {"geforce", "GeFOwOce"},
        {"raspberry", "Nyasberry"},
        {"broadcom", "Bwoadcom"},
        {"motorola", "MotOwOwa"},
        {"proliant", "ProLinyant"},
        {"poweredge", "POwOwEdge"},
        {"apple", "Nyapple"},
        {"electronic", "ElectrOwOnic"},
        {"processor", "Pwocessow"},
        {"microsoft", "MicOwOsoft"},
        {"ryzen", "Wyzen"},
        {"advanced", "Adwanced"},
        {"micro", "Micwo"},
        {"devices", "Dewices"},
        {"inc.", "Nyanc."},
        {"lucienne", "Lucienyan"},
        {"tuxedo", "TUWUXEDO"},
        {"aura", "Uwura"},

        {"linux", "linuwu"},
        {"alpine", "Nyalpine"},
        {"amogos", "AmogOwOS"},
        {"android", "Nyandroid"},
        {"arch", "Nyarch Linuwu"},

        {"arcolinux", "ArcOwO Linuwu"},

        {"artix", "Nyartix Linuwu"},
        {"debian", "Debinyan"},

        {"devuan", "Devunyan"},

        {"deepin", "Dewepyn"},
        {"endeavouros", "endeavOwO"},
        {"fedora", "Fedowa"},
        {"femboyos", "FemboyOWOS"},
        {"gentoo", "GentOwO"},
        {"gnu", "gnUwU"},
        {"guix", "gnUwU gUwUix"},
        {"linuxmint", "LinUWU Miwint"},
        {"manjaro", "Myanjawo"},
        {"manjaro-arm", "Myanjawo AWM"},
        {"neon", "KDE NeOwOn"},
        {"nixos", "nixOwOs"},
        {"opensuse-leap", "OwOpenSUSE Leap"},
        {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
        {"pop", "PopOwOS"},
        {"raspbian", "RaspNyan"},
        {"rocky", "Wocky Linuwu"},
        {"slackware", "Swackwawe"},
        {"solus", "sOwOlus"},
        {"ubuntu", "Uwuntu"},
        {"void", "OwOid"},
        {"xerolinux", "xuwulinux"},

        // BSD
        {"freebsd", "FweeBSD"},
        {"openbsd", "OwOpenBSD"},

        // Apple family
        {"macos", "macOwOS"},
        {"ios", "iOwOS"},

        // Windows
        {"windows", "WinyandOwOws"},
    };
    std::string input_text =
        "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel "
        "processor";
    constexpr std::string_view expected =
        "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; "
        "AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test8() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"brew-cask", "bwew-cawsk"},
        {"brew-cellar", "bwew-cewwaw"},
        {"emerge", "emewge"},
        {"flatpak", "fwatpakkies"},
        {"pacman", "pacnyan"},
        {"port", "powt"},
        {"rpm", "rawrpm"},
        {"snap", "snyap"},
        {"zypper", "zyppew"},

        {"lenovo", "LenOwO"},
        {"cpu", "CPUwU"},
        {"core", "Cowe"},
        {"gpu", "GPUwU"},
        {"graphics", "Gwaphics"},
        {"corporation", "COwOpowation"},
        {"nvidia", "NyaVIDIA"},
        {"mobile", "Mwobile"},
        {"intel", "Inteww"},
        {"radeon", "Radenyan"},
        {"geforce", "GeFOwOce"},
        {"raspberry", "Nyasberry"},
        {"broadcom", "Bwoadcom"},
        {"motorola", "MotOwOwa"},
        {"proliant", "ProLinyant"},
        {"poweredge", "POwOwEdge"},
        {"apple", "Nyapple"},
        {"electronic", "ElectrOwOnic"},
        {"processor", "Pwocessow"},
        {"microsoft", "MicOwOsoft"},
        {"ryzen", "Wyzen"},
        {"advanced", "Adwanced"},
        {"micro", "Micwo"},
        {"devices", "Dewices"},
        {"inc.", "Nyanc."},
        {"lucienne", "Lucienyan"},
        {"tuxedo", "TUWUXEDO"},
        {"aura", "Uwura"},

        {"linux", "linuwu"},
        {"alpine", "Nyalpine"},
        {"amogos", "AmogOwOS"},
        {"android", "Nyandroid"},
        {"arch", "Nyarch Linuwu"},

        {"arcolinux", "ArcOwO Linuwu"},

        {"artix", "Nyartix Linuwu"},
        {"debian", "Debinyan"},

        {"devuan", "Devunyan"},

        {"deepin", "Dewepyn"},
        {"endeavouros", "endeavOwO"},
        {"fedora", "Fedowa"},
        {"femboyos", "FemboyOWOS"},
        {"gentoo", "GentOwO"},
        {"gnu", "gnUwU"},
        {"guix", "gnUwU gUwUix"},
        {"linuxmint", "LinUWU Miwint"},
        {"manjaro", "Myanjawo"},
        {"manjaro-arm", "Myanjawo AWM"},
        {"neon", "KDE NeOwOn"},
        {"nixos", "nixOwOs"},
        {"opensuse-leap", "OwOpenSUSE Leap"},
        {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
        {"pop", "PopOwOS"},
        {"raspbian", "RaspNyan"},
        {"rocky", "Wocky Linuwu"},
        {"slackware", "Swackwawe"},
        {"solus", "sOwOlus"},
        {"ubuntu", "Uwuntu"},
        {"void", "OwOid"},
        {"xerolinux", "xuwulinux"},

        // BSD
        {"freebsd", "FweeBSD"},
        {"openbsd", "OwOpenBSD"},

        // Apple family
        {"macos", "macOwOS"},
        {"ios", "iOwOS"},

        // Windows
        {"windows", "WinyandOwOws"},
    };
    std::string input_text =
        "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel "
        "processor";
    constexpr std::string_view expected =
        "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; "
        "AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test9() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"abc", "def"},
        {"ghi", "jkz"},
    };
    std::string input_text              = "Abghciashjdhwdjahwdjhabdabanabwc";
    constexpr std::string_view expected = "Abghciashjdhwdjahwdjhabdabanabwc";
    std::string input_text_copy(input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy, expected,
                                 /*replace_all_occurances=*/false));
    assert(input_text_copy == input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy, expected,
                                 /*replace_all_occurances=*/true));
    assert(input_text_copy == input_text);
}

void test10() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"abc", "def"},
        {"ghi", "jkz"},
    };
    std::string input_text                                     = "Qghiabcabcghiabc";
    constexpr std::string_view expected_after_one_replacement  = "Qjkzabcabcghiabc";
    constexpr std::string_view expected_after_all_replacements = "Qjkzdefdefjkzdef";
    std::string input_text_copy(input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy,
                                 expected_after_one_replacement,
                                 /*replace_all_occurances=*/false));
    assert(test_replacing_actrie(patterns_with_replacements, input_text,
                                 expected_after_all_replacements,
                                 /*replace_all_occurances=*/true));
}

}  // namespace replacing_actrie_tests

}  // namespace

int main() {
    actrie_tests::test0();
    actrie_tests::test1();
    actrie_tests::test2();
    replacing_actrie_tests::test0();
    replacing_actrie_tests::test1();
    replacing_actrie_tests::test2();
    replacing_actrie_tests::test3();
    replacing_actrie_tests::test4();
    replacing_actrie_tests::test5();
    replacing_actrie_tests::test6();
    replacing_actrie_tests::test7();
    replacing_actrie_tests::test8();
    replacing_actrie_tests::test9();
    replacing_actrie_tests::test10();
    return 0;
}
