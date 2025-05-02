#include <assert.h>  // assert
#include <stdio.h>   // printf

#include "actrie.h"

void run_test(const char* (*patterns_with_replacements)[2],
              size_t size,
              char input_text[],
              size_t input_text_size,
              const char expected[],
              bool replace_all_occurances,
              uint32_t test_number) {
    struct actrie_t t;
    actrie_t_ctor(&t);

    actrie_t_reserve_patterns(&t, size);
    for (size_t i = 0; i < size; i++) {
        actrie_t_add_pattern(&t, patterns_with_replacements[i][0],
                             patterns_with_replacements[i][1]);
    }

    bool result = true;
    if (t.words_replacement.size != t.words_lengths.size || t.words_replacement.size != size) {
        result &= false;
        goto cleanup;
    }

    actrie_t_compute_links(&t);

    for (size_t i = 0; i < size; i++) {
        if (!actrie_t_contains_pattern(&t, patterns_with_replacements[i][0])) {
            result = false;
            goto cleanup;
        }
    }

    input_text[input_text_size] = '\0';
    if (replace_all_occurances) {
        actrie_t_replace_all_occurances(&t, input_text);
    } else {
        actrie_t_replace_first_occurance(&t, input_text);
    }
    result &= (strcmp(input_text, expected) == 0);

cleanup:
    actrie_t_dtor(&t);
    printf((result ? "test %u passed\n" : "test %u not passed\n"), test_number);
}

[[maybe_unused]] static void test0_short() {
    const char* patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"fasb", "xfasbx"},
    };
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ababcdacafaasbfasbabcc  ";
    const char exptected[] = "cdcdcdacafccsbxfasbxcdcc";
    const size_t input_text_size = 22;  // after 'c'

    run_test(patterns_with_replacements, size, input_text, input_text_size, exptected, true, 0);
}

[[maybe_unused]] static void test1_short() {
    const char* patterns_with_replacements[][2] = {
        {"ab", "cd"}, {"ba", "dc"}, {"aa", "cc"}, {"bb", "dd"}, {"xfasbx", "fasb"},
    };
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ababcdacafaasbxfasbxabcc";
    const char exptected[] = "cdcdcdacafccsbfasbcdcc";
    const size_t input_text_size = 24;  // 'c'

    run_test(patterns_with_replacements, size, input_text, input_text_size, exptected, true, 1);
}

[[maybe_unused]] static void test2_short() {
    const char* patterns_with_replacements[][2] = {
        {"LM", "0000"}, {"GHI", "111111"}, {"BCD", "2222222"}, {"nop", "3333"}, {"jk", "44444"}};
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ABCDEFGHIJKLMNOP             ";
    const char exptected[] = "A2222222EF1111114444400003333";
    const size_t input_text_size = 16;  // after 'P'

    run_test(patterns_with_replacements, size, input_text, input_text_size, exptected, true, 2);
}

[[maybe_unused]] static void test3_short() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "111111111111111111111111"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "no"}};
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ABCDEFGHIJKLMNOP                      ";
    const char expected[] = "111111111111111111111111cdefGHIjkLMnoP";
    const size_t input_text_size = 16;  // after 'P'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 3);
}

[[maybe_unused]] static void test4_short() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "ef"}, {"JK", "jk"}, {"NO", "111111111111111111111111"}};
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ABCDEFGHIJKLMNOP                      ";
    const char expected[] = "abcdefGHIjkLM111111111111111111111111P";
    const size_t input_text_size = 16;  // after 'P'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 4);
}

[[maybe_unused]] static void test5_short() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "ab"}, {"CD", "cd"}, {"EF", "111111111111111111111111"}, {"JK", "jk"}, {"NO", "no"}};
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "ABCDEFGHIJKLMNOP                      ";
    const char expected[] = "abcd111111111111111111111111GHIjkLMnoP";
    const size_t input_text_size = 16;  // after 'P'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 5);
}

[[maybe_unused]] static void test6_short() {
    const char* patterns_with_replacements[][2] = {{"kernel", "Kewnel"},
                                                   {"linux", "Linuwu"},
                                                   {"debian", "Debinyan"},
                                                   {"ubuntu", "Uwuntu"},
                                                   {"windows", "WinyandOwOws"}};
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "linux kernel; debian os; ubuntu os; windows os        ";
    const char expected[] = "Linuwu Kewnel; Debinyan os; Uwuntu os; WinyandOwOws os";
    const size_t input_text_size = 46;  // after 's'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 6);
}

[[maybe_unused]] static void test7_long() {
    const char* patterns_with_replacements[][2] = {
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
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] =
        "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel "
        "processor                             ";
    const char expected[] =
        "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; "
        "AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    const size_t input_text_size = 97;  // after 'r'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 7);
}

[[maybe_unused]] static void test8_long() {
    const char* patterns_with_replacements[][2] = {
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
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] =
        "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel "
        "processor                             ";
    const char expected[] =
        "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; "
        "AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    const size_t input_text_size = 97;  // after 'r'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 8);
}

[[maybe_unused]] static void test9_short() {
    const char* patterns_with_replacements[][2] = {
        {"abc", "def"},
        {"ghi", "jkz"},
    };
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "Abghciashjdhwdjahwdjhabdabanabwc";
    const char expected[] = "Abghciashjdhwdjahwdjhabdabanabwc";
    const size_t input_text_size = 32;  // after 'c'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, true, 9);
}

[[maybe_unused]] static void test10_short() {
    const char* patterns_with_replacements[][2] = {
        {"abc", "def"},
        {"ghi", "jkz"},
    };
    const size_t size = sizeof(patterns_with_replacements) / sizeof(patterns_with_replacements[0]);

    char input_text[] = "Qghiabcabcghiabc";
    const char expected[] = "Qjkzabcabcghiabc";
    const size_t input_text_size = 16;  // after 'c'
    run_test(patterns_with_replacements, size, input_text, input_text_size, expected, false, 10);
}

int main() {
    test0_short();
    test1_short();
    test2_short();
    test3_short();
    test4_short();
    test5_short();
    test6_short();
    test7_long();
    test8_long();
    test9_short();
    test10_short();
}
