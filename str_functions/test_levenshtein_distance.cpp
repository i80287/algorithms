#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <ranges>

#include "../misc/config_macros.hpp"
#include "levenshtein_distance.hpp"

namespace {

constexpr bool verify() {
    using str_tools::levenshtein_distance;

    struct TestCase {
        std::string_view s1;
        std::string_view s2;
        uint32_t test_answer;
    };

    // clang-format off
    constexpr TestCase tests[] = {
        {"head", "participant", 10},
        {"expert", "after", 4},
        {"type", "physical", 7},
        {"author", "focus", 6},
        {"outside", "health", 7},
        {"N", "slT:[N>.WNY#ALD?qiiO:+D=zqBpdseFb", 32},
        {":knFkfsC@!JFOPgXA^.S>}mA=gf&U.I_zndVj", "fc#z", 35},
        {"UOL=h", "bAZZo?T&Dri}?VtkrwIb?!XoO~~JLv.", 29},
        {"ghukgHA#EM~,<B<C!tfOyS>M", "!Ck_.ZgOum?rhDud~X_YU-n:?zUWh", 27},
        {"~pt,UxYE_=Iji]EUgPV%T|#", ",|<}Q:nO", 22},
        {"[.{PSOC_{-p?rI:ZVCh}hg&sSz&<r<BON{V|%:[Mx^gLzQ>zyPqH><rhYEM<x%W", "Y]wMea", 62},
        {"^gxI=mpHUxB+JgOVFmz^Mdo#+V*[%pWWN{Ad{z@Ng=BKguD{zV|", "XpjzjP[]aY>I=@TQKdrkTcGbo[BxO>JN<", 48},
        {"nzxpKjz$W[Ib", ",ge$RPLvsC]", 12},
        {"Zsp}AUt*C", "~$:kicE?wTUJ", 12},
        {"rD", "fAs:%bHF->", 10},
        {"@]H&U==+Di", "bns|dl,", 10},
        {"kvpmsprIdYL-ls<:+{nMhEKr<o-Ttz$Q$P$JZ@ElTVftTzJo~V?PzCH-$CTm]j{svhI:C%>Rlwe=s=V!$]OjpUasrDlGNd", "KBO!f", 92},
        {"tG,oQpDLm.-:oSLvx]Nh[q^-LrnjM.i}j!LzmxNujj#H=Y", "ZrqiYtiun}IkK}[iTD<mP*Nu:Pd", 42},
        {"sunday", "sudnay", 2},
        {"sunday", "saturday", 3},
        {"cat", "cats", 1},
        {"cats", "cat", 1},
    };
    // clang-format on

    constexpr auto kMaxSizeFn = [](const TestCase& test) constexpr noexcept {
        return std::max(test.s1.size(), test.s2.size());
    };
    constexpr size_t kMaxSize =
        kMaxSizeFn(std::ranges::max(tests, [&](const TestCase& t1, const TestCase& t2) constexpr noexcept {
            return kMaxSizeFn(t1) < kMaxSizeFn(t2);
        }));
    constexpr size_t kBufferSize = kMaxSize + 1;
    std::array<char, kBufferSize> buffer1{};
    std::array<char, kBufferSize> buffer2{};

    for (const auto [s1, s2, ans] : tests) {
        if (levenshtein_distance(s1, s2) != ans) {
            return false;
        }
        if (levenshtein_distance(s2, s1) != ans) {
            return false;
        }

        assert(s1.size() <= buffer2.size());
        std::ranges::copy(s1, buffer1.begin());
        std::reverse(buffer1.begin(), buffer1.begin() + s1.size());
        const std::string_view s1_rev_view(buffer1.data(), s1.size());

        assert(s2.size() <= buffer2.size());
        std::ranges::copy(s2, buffer2.begin());
        std::reverse(buffer2.begin(), buffer2.begin() + s2.size());
        const std::string_view s2_rev_view(buffer2.data(), s2.size());

        if (levenshtein_distance(s1_rev_view, s2_rev_view) != ans) {
            return false;
        }
        if (levenshtein_distance(s2_rev_view, s1_rev_view) != ans) {
            return false;
        }

        if (config::is_constant_evaluated()) {
            break;
        }
    }

    return true;
}

}  // namespace

int main() {
#if CONFIG_VECTOR_SUPPORTS_CONSTEXPR_OPERATIONS
    static_assert(verify(), "");
#endif
    assert(verify());
}
