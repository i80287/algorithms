#include <cstdint>  // std::int32_t, std::uint16_t, std::uint32_t, std::int32_t
#include <string>   // std::char_traits
#include <vector>   // std::vector

namespace string_switch_impl {

using std::int32_t;
using std::size_t;
using std::uint16_t;
using std::uint32_t;

/// @brief std::min is not used in order not to include additional header
/// (<algorithm>) just for min/max
/// @param a
/// @param b
consteval int32_t min(int32_t a, int32_t b) noexcept { return a <= b ? a : b; }

/// @brief std::min is not used in order not to include additional header
/// (<algorithm>) just for min/max
/// @param a
/// @param b
consteval int32_t max(int32_t a, int32_t b) noexcept { return a >= b ? a : b; }

template <size_t N>
struct CompileTimeStringLiteral {
    static_assert(N > 0);

    consteval CompileTimeStringLiteral(const char (&str)[N]) {
        std::char_traits<char>::copy(value, str, N);
    }

    consteval size_t size() const noexcept { return N - 1; }

    char value[N];
};

template <size_t kNodesCount, uint32_t kMinChar, uint32_t kMaxChar>
class StringSwitchImpl {
    static_assert(kMinChar <= kMaxChar);
    static constexpr size_t kAlphabetSize = size_t(kMaxChar - kMinChar) + 1;

    struct {
        uint32_t children[kAlphabetSize] = {};
        uint16_t string_index = uint16_t(-1);
        bool terminal = false;
    } nodes_[kNodesCount] = {};

    consteval void AddPattern(size_t, size_t) noexcept {}

    template <class... Args>
    consteval void AddPattern(size_t first_free_node_index, size_t string_index,
                              const auto& string, Args... args) noexcept {
        size_t current_node = 0;
        for (size_t i = 0; i < string.size(); i++) {
            uint32_t c = uint8_t(string.value[i]);
            size_t index = c - kMinChar;
            size_t child_index = nodes_[current_node].children[index];
            if (child_index != 0) {
                current_node = child_index;
            } else {
                nodes_[current_node].children[index] =
                    uint32_t(first_free_node_index);
                current_node = first_free_node_index;
                first_free_node_index++;
            }
        }

        nodes_[current_node].terminal = true;
        // Value is overwritten if the same patterns are added to the trie
        nodes_[current_node].string_index = uint16_t(string_index);
        AddPattern(first_free_node_index, string_index + 1, args...);
    }

public:
    static constexpr uint32_t kDefaultSwitch = uint32_t(-1);

    template <class... Args>
    consteval StringSwitchImpl(Args... args) noexcept {
        static_assert(sizeof...(Args) <= UINT16_MAX);
        // 0 is root index, start from 1
        AddPattern(size_t(1), size_t(0), args...);
    }

    constexpr uint32_t Switch(const char* str) const noexcept {
        if (str == nullptr) [[unlikely]] {
            return kDefaultSwitch;
        }

        size_t current_node = 0;
        for (uint32_t c = 0; (c = uint8_t(*str)) != '\0'; str++) {
            size_t index = c - kMinChar;
            if (index >= kAlphabetSize) {
                return kDefaultSwitch;
            }

            size_t next_node = nodes_[current_node].children[index];
            if (next_node != 0) {
                current_node = next_node;
            } else {
                return kDefaultSwitch;
            }
        }

        return nodes_[current_node].terminal ? nodes_[current_node].string_index
                                             : kDefaultSwitch;
    }
};

template <CompileTimeStringLiteral FirstString,
          CompileTimeStringLiteral... Strings>
consteval int32_t CountMinChar() noexcept {
    // not FirstString.value[0] because it can be '\0'
    int32_t min_char = INT32_MAX;
    constexpr size_t len = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        int32_t chr = uint8_t(FirstString.value[i]);
        if (chr < min_char) {
            min_char = chr;
        }
    }
    if constexpr (sizeof...(Strings) != 0) {
        return std::min(min_char, CountMinChar<Strings...>());
    } else {
        return min_char;
    }
}

template <CompileTimeStringLiteral FirstString,
          CompileTimeStringLiteral... Strings>
consteval int32_t CountMaxChar() noexcept {
    int32_t max_char = INT32_MIN;
    constexpr size_t len = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        int32_t chr = uint8_t(FirstString.value[i]);
        if (chr > max_char) {
            max_char = chr;
        }
    }
    if constexpr (sizeof...(Strings) != 0) {
        return std::max(max_char, CountMaxChar<Strings...>());
    } else {
        return max_char;
    }
}

template <uint32_t IndexesSize>
struct CountingImplNode {
    static_assert(IndexesSize > 0);
    uint32_t edges[IndexesSize] = {};
};

template <uint32_t MinChar, uint32_t MaxChar,
          CompileTimeStringLiteral FirstString,
          CompileTimeStringLiteral... Strings>
consteval size_t CountNodesImpl(
    std::vector<CountingImplNode<MaxChar - MinChar + 1>>& nodes) {
    uint32_t current_node_index = 0;
    size_t pattern_index = 0;
    constexpr size_t pattern_size = FirstString.size();

    for (; pattern_index != pattern_size; ++pattern_index) {
        uint32_t sigma = uint8_t(FirstString.value[pattern_index]);
        uint32_t next_node_index =
            nodes.at(current_node_index).edges[sigma - MinChar];
        if (next_node_index != 0) {
            current_node_index = next_node_index;
        } else {
            break;
        }
    }

    size_t lasted_max_length = pattern_size - pattern_index;
    nodes.reserve(nodes.size() + lasted_max_length);

    /*
     * Inserts substring [i..length - 1] of pattern if i < length (<=> i !=
     * length) If i == length, then for cycle will no execute
     */
    for (uint32_t new_node_index = uint32_t(nodes.size());
         pattern_index != pattern_size; ++pattern_index) {
        uint32_t sigma = uint8_t(FirstString.value[pattern_index]);
        nodes.emplace_back();
        nodes[current_node_index].edges[sigma - MinChar] = new_node_index;
        current_node_index = new_node_index++;
    }

    if constexpr (sizeof...(Strings) != 0) {
        return CountNodesImpl<MinChar, MaxChar, Strings...>(nodes);
    } else {
        return nodes.size();
    }
}

template <uint32_t MinChar, uint32_t MaxChar,
          CompileTimeStringLiteral... Strings>
consteval size_t CountNodes() {
    std::vector<CountingImplNode<MaxChar - MinChar + 1>> nodes(size_t(1));
    return CountNodesImpl<MinChar, MaxChar, Strings...>(nodes);
}

}  // namespace string_switch_impl

template <string_switch_impl::CompileTimeStringLiteral... Strings>
consteval auto StringSwitch() {
    constexpr int32_t kMinChar = string_switch_impl::CountMinChar<Strings...>();
    constexpr int32_t kMaxChar = string_switch_impl::CountMaxChar<Strings...>();
    constexpr bool kEmptyStrings1 = kMinChar == INT32_MAX;
    constexpr bool kEmptyStrings2 = kMaxChar == INT32_MIN;
    static_assert(kEmptyStrings1 == kEmptyStrings2, "impl error");

    if constexpr (kEmptyStrings1) {
        return string_switch_impl::StringSwitchImpl<size_t(1), uint32_t('\0'),
                                                    uint32_t('\0')>(Strings...);
    } else {
        static_assert(kMinChar > 0 && kMaxChar > 0, "impl error");
        constexpr size_t kNodesCount =
            string_switch_impl::CountNodes<uint32_t(kMinChar),
                                           uint32_t(kMaxChar), Strings...>();
        return string_switch_impl::StringSwitchImpl<
            kNodesCount, uint32_t(kMinChar), uint32_t(kMaxChar)>(Strings...);
    }
}
