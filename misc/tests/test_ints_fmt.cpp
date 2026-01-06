// clang-format off
#include "../ints_fmt.hpp"
// clang-format on

#include <algorithm>
#include <cassert>
#include <limits>

#include "test_tools.hpp"

// clang-format off
#include "../ints_fmt.hpp"
// clang-format on

namespace {

template <class IntType>
class FormatterTestSuite final {
public:
    static void run() {
        test_tools::log_tests_started();
        test_ranges();
    }

private:
    static void test_ranges() {
        using limits = std::numeric_limits<IntType>;

        static_assert(limits::max() > 0, "");
        static constexpr auto kRange = limits::max() < 2500 ? limits::max() / 2 : static_cast<IntType>(2500);

        for (IntType value = limits::min(); value <= limits::min() + kRange; value++) {
            check_fmt(value);
        }

        if constexpr (limits::min() != 0) {
            for (IntType value = 0; value <= kRange; value++) {
                check_fmt(value);
            }
        }

        for (IntType value = limits::max(); value >= limits::max() - kRange; value--) {
            check_fmt(value);
        }
    }

    static void check_fmt(const IntType value) {
        using ExtT = std::conditional_t<sizeof(IntType) < sizeof(int),
                                        std::conditional_t<std::is_signed_v<IntType>, int, unsigned>, IntType>;
        assert(ints_fmt::Formatter<IntType>{value}.as_string_view() == std::to_string(ExtT{value}));
    }
};

void run_tests() {
    FormatterTestSuite<std::int8_t>::run();
    FormatterTestSuite<std::uint8_t>::run();
    FormatterTestSuite<std::int16_t>::run();
    FormatterTestSuite<std::uint16_t>::run();
    FormatterTestSuite<std::int32_t>::run();
    FormatterTestSuite<std::uint32_t>::run();
    FormatterTestSuite<std::int32_t>::run();
    FormatterTestSuite<std::uint32_t>::run();
    FormatterTestSuite<std::int64_t>::run();
    FormatterTestSuite<std::uint64_t>::run();
}

}  // namespace

int main() {
    run_tests();
}
