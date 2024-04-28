#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace string_switch_impl {

using int32_t  = std::int32_t;
using size_t   = std::size_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using uint8_t  = std::uint8_t;

inline constexpr size_t kMaxStringViewLength = 200;

template <size_t N = kMaxStringViewLength>
struct CompileTimeStringLiteral {
    static_assert(N > 0);

    consteval CompileTimeStringLiteral(std::string_view str) : length(str.size()) {
        // Change kMaxStringViewLength if you are using
        //  very long strings in the StringSwitch.
        [[maybe_unused]] const auto string_view_size_check = 0 / (str.size() <= std::size(value));
        std::char_traits<char>::copy(value, str.data(), str.size());
    }

    consteval CompileTimeStringLiteral(const char (&str)[N]) : length(N - 1) {
        std::char_traits<char>::copy(value, str, N);
    }

    consteval size_t size() const noexcept {
        return length;
    }

    char value[N]{};
    const size_t length;
};

template <size_t NodesCount, uint32_t MinChar, uint32_t MaxChar,
          size_t StringsCount>
class StringSwitchImpl {
    static_assert(MinChar <= MaxChar &&
                  MaxChar <= std::numeric_limits<uint8_t>::max(), "impl error");

    using str_index_t = uint32_t;

    static constexpr size_t kAlphabetSize = size_t(MaxChar - MinChar) + 1;
    static constexpr str_index_t kNonTerminalStringIndex =
        std::numeric_limits<str_index_t>::max();

    struct TrieNodeImpl {
        uint32_t children[kAlphabetSize] = {};
        str_index_t string_index         = kDefaultSwitch;
    };
    TrieNodeImpl nodes_[NodesCount] = {};

    static constexpr uint32_t CharToIndex(uint8_t chr) noexcept {
        return static_cast<uint32_t>(chr) - MinChar;
    }

    template <class... Args>
    consteval void AddPattern(size_t first_free_node_index,
                              str_index_t string_index, const auto& string,
                              Args... args) noexcept {
        size_t current_node = 0;
        for (size_t i = 0; i < string.size(); i++) {
            size_t index =
                CharToIndex(static_cast<uint8_t>(string.value[i]));
            size_t child_index = nodes_[current_node].children[index];
            if (child_index != 0) {
                current_node = child_index;
            } else {
                nodes_[current_node].children[index] =
                    static_cast<uint32_t>(first_free_node_index);
                current_node = first_free_node_index;
                first_free_node_index++;
            }
        }

        // Value is overwritten if the same patterns are added to the trie
        nodes_[current_node].string_index = string_index;
        if constexpr (sizeof...(args) > 0) {
            AddPattern(first_free_node_index, string_index + 1, args...);
        }
    }

public:
    static constexpr uint32_t kDefaultSwitch =
        static_cast<uint32_t>(StringsCount);
    static constexpr char kMinChar = static_cast<char>(MinChar);
    static constexpr char kMaxChar = static_cast<char>(MaxChar);

    template <class... Args>
    consteval StringSwitchImpl(Args... args) noexcept {
        // 0 is the root index, start from 1
        constexpr size_t kInitialFirstFreeNodeIndex = 1;
        AddPattern(kInitialFirstFreeNodeIndex, str_index_t(0), args...);
    }

    [[nodiscard]] consteval uint32_t operator()(
        std::nullptr_t) const noexcept = delete;

    [[nodiscard]] constexpr uint32_t operator()(
        std::string_view str) const noexcept {
        return operator()(str.data());
    }

    [[nodiscard]] constexpr uint32_t operator()(
        const char* str) const noexcept {
        if (str == nullptr) [[unlikely]] {
            return kDefaultSwitch;
        }

        size_t current_node = 0;
        for (uint8_t c = 0; (c = static_cast<uint8_t>(*str)) != '\0';
             str++) {
            size_t index = CharToIndex(c);
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

        // If node is not terminal it is
        /// already equal to kDefaultSwitch.
        return nodes_[current_node].string_index;
    }
};

/// @brief std::min is not used in order not to include additional header
/// (<algorithm>) just for min/max
/// @param a
/// @param b
/// @return
consteval int32_t min(int32_t a, int32_t b) noexcept {
    return a <= b ? a : b;
}

/// @brief std::min is not used in order not to include additional header
/// (<algorithm>) just for min/max
/// @param a
/// @param b
/// @return
consteval int32_t max(int32_t a, int32_t b) noexcept {
    return a >= b ? a : b;
}

struct MinMaxChars {
    int32_t min_char;
    int32_t max_char;
};

template <CompileTimeStringLiteral FirstString,
          CompileTimeStringLiteral... Strings>
consteval MinMaxChars FindMinMaxChars() noexcept {
    // not FirstString.value[0] because it can be '\0'
    int32_t min_char     = std::numeric_limits<int32_t>::max();
    int32_t max_char     = std::numeric_limits<int32_t>::min();
    constexpr size_t len = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        const int32_t chr = static_cast<uint8_t>(FirstString.value[i]);
        min_char          = min(min_char, chr);
        max_char          = max(max_char, chr);
    }
    if constexpr (sizeof...(Strings) > 0) {
        const MinMaxChars mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char = min(min_char, mn_mx_chars.min_char);
        max_char = max(max_char, mn_mx_chars.max_char);
    }

    return {
        .min_char = min_char,
        .max_char = max_char,
    };
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
    uint32_t current_node_index   = 0;
    size_t pattern_index          = 0;
    constexpr size_t pattern_size = FirstString.size();

    for (; pattern_index != pattern_size; ++pattern_index) {
        uint32_t sigma =
            static_cast<uint8_t>(FirstString.value[pattern_index]);
        uint32_t next_node_index =
            nodes[current_node_index].edges[sigma - MinChar];
        if (next_node_index != 0) {
            current_node_index = next_node_index;
        } else {
            break;
        }
    }

    size_t lasted_max_length = pattern_size - pattern_index;
    nodes.reserve(nodes.size() + lasted_max_length);

    for (uint32_t new_node_index = static_cast<uint32_t>(nodes.size());
         pattern_index != pattern_size; ++pattern_index) {
        uint32_t sigma =
            static_cast<uint8_t>(FirstString.value[pattern_index]);
        nodes.emplace_back();
        nodes[current_node_index].edges[sigma - MinChar] = new_node_index;
        current_node_index = new_node_index++;
    }

    if constexpr (sizeof...(Strings) > 0) {
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
    constexpr string_switch_impl::MinMaxChars kMinMaxChars =
        string_switch_impl::FindMinMaxChars<Strings...>();

    constexpr bool kEmptyStrings1 =
        kMinMaxChars.min_char == std::numeric_limits<int32_t>::max();
    constexpr bool kEmptyStrings2 =
        kMinMaxChars.max_char == std::numeric_limits<int32_t>::min();
    static_assert(kEmptyStrings1 == kEmptyStrings2, "impl error");

    if constexpr (kEmptyStrings1) {
        return string_switch_impl::StringSwitchImpl<
            size_t(1), uint32_t('\0'), uint32_t('\0'), sizeof...(Strings)>(
            Strings...);
    } else {
        static_assert(
            kMinMaxChars.min_char > 0 && kMinMaxChars.max_char > 0,
            "impl error");

        constexpr uint32_t kMinUChar =
            static_cast<uint32_t>(kMinMaxChars.min_char);
        constexpr uint32_t kMaxUChar =
            static_cast<uint32_t>(kMinMaxChars.max_char);

        constexpr size_t kNodesCount =
            string_switch_impl::CountNodes<kMinUChar, kMaxUChar,
                                           Strings...>();
        return string_switch_impl::StringSwitchImpl<
            kNodesCount, kMinUChar, kMaxUChar, sizeof...(Strings)>(
            Strings...);
    }
}
