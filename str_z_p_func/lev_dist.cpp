#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

template <uint32_t ReplacementCost = 1, uint32_t DeletionCost = 1,
          uint32_t InsertionCost = 1>
constexpr uint32_t ldist(const std::string_view s1,
                         const std::string_view s2) noexcept {
    std::vector<uint32_t> dp1(s2.size() + 1);
    std::vector<uint32_t> dp2(s2.size() + 1);
    for (size_t j = 0; j <= s2.size(); j++) {
        dp1[j] = uint32_t(j);
    }

    for (size_t i = 1; i <= s1.size(); i++) {
        dp2[0] = uint32_t(i);
        for (size_t j = 1; j <= s2.size(); j++) {
            dp2[j] =
                std::min(dp1[j - 1] + uint32_t(s1[i - 1] != s2[j - 1]) *
                                          ReplacementCost,
                         std::min(dp1[j] + DeletionCost,
                                  dp2[j - 1] + InsertionCost));
        }
        std::swap(dp1, dp2);
    }

    return dp1[s2.size()];
}

#include <algorithm>
#include <array>
#include <ranges>

consteval bool verify() {
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

    constexpr auto kMaxSizeFn =
        [](const TestCase& test) constexpr noexcept {
            return std::max(test.s1.size(), test.s2.size());
        };
    constexpr size_t kMaxSize    = kMaxSizeFn(std::ranges::max(
        tests,
        [&](const TestCase& t1, const TestCase& t2) constexpr noexcept {
            return kMaxSizeFn(t1) < kMaxSizeFn(t2);
        }));
    constexpr size_t kBufferSize = kMaxSize + 1;
    std::array<char, kBufferSize> buffer1;
    std::array<char, kBufferSize> buffer2;

    for (auto [s1, s2, ans] : tests) {
        if (ldist(s1, s2) != ans) {
            return false;
        }
        if (ldist(s2, s1) != ans) {
            return false;
        }

        std::ranges::copy(s1, buffer1.begin());
        buffer1[s1.size()] = '\0';
        std::reverse(buffer1.begin(), buffer1.begin() + s1.size());
        s1 = std::string_view(buffer1.data(), s1.size());

        std::ranges::copy(s2, buffer2.begin());
        buffer2[s2.size()] = '\0';
        std::reverse(buffer2.begin(), buffer2.begin() + s2.size());
        s2 = std::string_view(buffer2.data(), s2.size());

        if (ldist(s1, s2) != ans) {
            return false;
        }
        if (ldist(s2, s1) != ans) {
            return false;
        }
    }
    return true;
}

int main() {
    static_assert(verify(), "");
}
