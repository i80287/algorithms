#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef NDEBUG
#error ("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

#include <cassert>

#include "actrie.hpp"

namespace actrie::tests {

namespace {

using std::size_t;

using OccurrencesContainer = std::vector<std::tuple<std::string_view, size_t, StoredPatternIndex>>;

template <size_t PatternsCount>
[[nodiscard]] bool test_actrie(const std::string_view (&patterns)[PatternsCount],
                               std::string_view text,
                               const OccurrencesContainer& expected_occurrences) {
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
    if (builder.PatternsCount() != PatternsCount) {
        return false;
    }
    const actrie::ACTrie t = std::move(builder).Build();
    for (const std::string_view pattern : patterns) {
        if (!t.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (t.PatternsCount() != PatternsCount) {
        return false;
    }

    const OccurrencesContainer found_occurrences = t.CollectAllSubstringsFromText<OccurrencesContainer>(text);
    return found_occurrences == expected_occurrences;
}

void test0() {
    constexpr std::string_view patterns[] = {
        "a", "ab", "ba", "aa", "bb", "fasb",
    };
    constexpr std::string_view text = "ababcdacafaasbfasbabcc";
    const OccurrencesContainer expected_occurrences = {
        {"a", 0, 0},  {"ab", 0, 1},    {"ba", 1, 2},  {"a", 2, 0},   {"ab", 2, 1},
        {"a", 6, 0},  {"a", 8, 0},     {"a", 10, 0},  {"aa", 10, 3}, {"a", 11, 0},
        {"a", 15, 0}, {"fasb", 14, 5}, {"ba", 17, 2}, {"a", 18, 0},  {"ab", 18, 1},
    };

    assert(test_actrie(patterns, text, expected_occurrences));
}

void test1() {
    constexpr std::string_view patterns[] = {
        "ABC",
        "CDE",
        "CDEF",
    };
    constexpr std::string_view text =
        "ABCDEFGHABCDEFGADCVABCDEBACBCBABDBEBCBABABBCDEBCBABDEBCABDBCBACABCDBEBACBCDEWBCBABCDE";
    const OccurrencesContainer expected_occurrences = {
        {"ABC", 0, 0},  {"CDE", 2, 1},  {"CDEF", 2, 2}, {"ABC", 8, 0},  {"CDE", 10, 1}, {"CDEF", 10, 2}, {"ABC", 19, 0},
        {"CDE", 21, 1}, {"CDE", 43, 1}, {"ABC", 63, 0}, {"CDE", 73, 1}, {"ABC", 80, 0}, {"CDE", 82, 1},
    };

    assert(test_actrie(patterns, text, expected_occurrences));
}

void test2() {
    constexpr std::string_view patterns[] = {
        "aba", "baca", "abacaba", "ccbba", "cabaaba",
    };
    constexpr std::string_view text =
        "ababcbbacbcabaabaacbacbbacbbabcbabcbcabaabaabcabaabacabaabacbabbbacbabacbabbacbcabacabcbcb"
        "acabaababcbabbacacbbcbcababbcbcbacabcabacbcababacababcbabccaababacabcbabcbacbabcabcbbababa"
        "caababababcbbcbcbcbcbcbababcbabcabccbbcbcbcabaabacabbacbabca";
    const OccurrencesContainer expected_occurrences = {
        {"aba", 0, 0},      {"aba", 11, 0},     {"cabaaba", 10, 4},  {"aba", 14, 0},     {"aba", 37, 0},
        {"cabaaba", 36, 4}, {"aba", 40, 0},     {"aba", 46, 0},      {"cabaaba", 45, 4}, {"aba", 49, 0},
        {"baca", 50, 1},    {"abacaba", 49, 2}, {"aba", 53, 0},      {"cabaaba", 52, 4}, {"aba", 56, 0},
        {"aba", 68, 0},     {"aba", 80, 0},     {"baca", 81, 1},     {"baca", 89, 1},    {"aba", 92, 0},
        {"cabaaba", 91, 4}, {"aba", 95, 0},     {"baca", 103, 1},    {"aba", 113, 0},    {"baca", 121, 1},
        {"aba", 127, 0},    {"aba", 133, 0},    {"aba", 135, 0},     {"baca", 136, 1},   {"abacaba", 135, 2},
        {"aba", 139, 0},    {"aba", 150, 0},    {"aba", 152, 0},     {"baca", 153, 1},   {"aba", 175, 0},
        {"aba", 177, 0},    {"baca", 178, 1},   {"aba", 182, 0},     {"aba", 184, 0},    {"aba", 186, 0},
        {"aba", 203, 0},    {"aba", 223, 0},    {"cabaaba", 222, 4}, {"aba", 226, 0},    {"baca", 227, 1},
    };

    assert(test_actrie(patterns, text, expected_occurrences));
}

}  // namespace

namespace replacing {

template <Case CaseOption = Case::Insensitive, size_t PatternsCount>
[[nodiscard]] bool test_replacing_actrie(const std::string_view (&patterns_with_replacements)[PatternsCount][2],
                                         std::string& input_text,
                                         const std::string_view expected,
                                         const bool replace_all_occurrences) {
    using BuilderType = actrie::ReplacingACTrieBuilder<
        /* AlphabetStart = */ '-',
        /* AlphabetEnd = */ '}', CaseOption>;
    auto builder = BuilderType::WithCapacity(PatternsCount);
    for (const auto& [pattern, replacement] : patterns_with_replacements) {
        if (!builder.AddPatternWithReplacement(pattern, std::string{replacement})) {
            return false;
        }
    }
    for (const auto& [pattern, _] : patterns_with_replacements) {
        if (!builder.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (builder.PatternsCount() != PatternsCount) {
        return false;
    }
    const actrie::ReplacingACTrie t = std::move(builder).Build();
    for (const auto& [pattern, _] : patterns_with_replacements) {
        if (!t.ContainsPattern(pattern)) {
            return false;
        }
    }
    if (t.PatternsCount() != PatternsCount) {
        return false;
    }

    if (replace_all_occurrences) {
        t.ReplaceAllOccurrences(input_text);
    } else {
        t.ReplaceFirstOccurrence(input_text);
    }
    return input_text == expected;
}

void test0() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"fasb", "xfasbx"},
    };
    std::string input_text = "ababcdacafaasbfasbabcc";
    const std::string expected = "cdcdcdacafccsbxfasbxcdcc";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test1() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"xfasbx", "fasb"},
    };
    std::string input_text = "ababcdacafaasbxfasbxabcc";
    const std::string expected = "cdcdcdacafccsbfasbcdcc";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test2() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"LM", "0000"}, {"GHI", "111111"}, {"BCD", "2222222"}, {"nop", "3333"}, {"jk", "44444"},
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const std::string expected = "A2222222EF1111114444400003333";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test3() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "111111111111111111111111"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "no"},
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const std::string expected = "111111111111111111111111cdefGHIjkLMnoP";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test4() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "111111111111111111111111"},
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const std::string expected = "abcdefGHIjkLM111111111111111111111111P";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test5() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "111111111111111111111111"}, {"JK", "jk"}, {"NO", "no"},
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const std::string expected = "abcd111111111111111111111111GHIjkLMnoP";
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected, true));
}

void test6() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"kernel", "Kewnel"}, {"linux", "Linuwu"},         {"debian", "Debinyan"},
        {"ubuntu", "Uwuntu"}, {"windows", "WinyandOwOws"},
    };
    std::string input_text = "linux kernel; debian os; ubuntu os; windows os";
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
    std::string input_text = "Abghciashjdhwdjahwdjhabdabanabwc";
    constexpr std::string_view expected = "Abghciashjdhwdjahwdjhabdabanabwc";
    std::string input_text_copy(input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy, expected,
                                 /*replace_all_occurrences=*/false));
    assert(input_text_copy == input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy, expected,
                                 /*replace_all_occurrences=*/true));
    assert(input_text_copy == input_text);
}

void test10() {
    constexpr std::string_view patterns_with_replacements[][2] = {
        {"abc", "def"},
        {"ghi", "jkz"},
    };
    std::string input_text = "Qghiabcabcghiabc";
    constexpr std::string_view expected_after_one_replacement = "Qjkzabcabcghiabc";
    constexpr std::string_view expected_after_all_replacements = "Qjkzdefdefjkzdef";
    std::string input_text_copy(input_text);
    assert(test_replacing_actrie(patterns_with_replacements, input_text_copy, expected_after_one_replacement,
                                 /*replace_all_occurrences=*/false));
    assert(test_replacing_actrie(patterns_with_replacements, input_text, expected_after_all_replacements,
                                 /*replace_all_occurrences=*/true));
}

}  // namespace replacing

}  // namespace actrie::tests

int main() {
    actrie::tests::test0();
    actrie::tests::test1();
    actrie::tests::test2();
    actrie::tests::replacing::test0();
    actrie::tests::replacing::test1();
    actrie::tests::replacing::test2();
    actrie::tests::replacing::test3();
    actrie::tests::replacing::test4();
    actrie::tests::replacing::test5();
    actrie::tests::replacing::test6();
    actrie::tests::replacing::test7();
    actrie::tests::replacing::test8();
    actrie::tests::replacing::test9();
    actrie::tests::replacing::test10();
    return 0;
}
