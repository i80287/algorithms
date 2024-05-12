#include <array>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace detail {

using int32_t  = std::int32_t;
using size_t   = std::size_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using uint8_t  = std::uint8_t;

inline constexpr size_t kMaxStringViewSize = 200;

template <size_t N = kMaxStringViewSize>
struct [[nodiscard]] CompileTimeStringLiteral {
    static_assert(N > 0);
    consteval CompileTimeStringLiteral(std::string_view str) noexcept : length(str.size()) {
        // Change kMaxStringViewSize if you are using
        //  very long strings in the StringSet / StringMap.
        [[maybe_unused]] const auto string_view_size_check =
            0 / (str.size() <= std::size(value));
        std::char_traits<char>::copy(value.data(), str.data(), str.size());
    }
    consteval CompileTimeStringLiteral(const char (&str)[N]) noexcept
        : length(str[N - 1] == '\0' ? N - 1 : N) {
        std::char_traits<char>::copy(value.data(), str, size());
    }
    [[nodiscard]] consteval size_t size() const noexcept {
        return length;
    }
    consteval char operator[](size_t index) const noexcept {
        return value[index];
    }
    consteval char& operator[](size_t index) noexcept {
        return value[index];
    }

    std::array<char, N> value{};
    const size_t length{};
};

namespace trie_tools {

struct TrieParamsType final {
    static constexpr uint32_t kRootNodeIndex = 0;
    uint32_t min_char;
    uint32_t max_char;
    size_t trie_alphabet_size = max_char - min_char + 1;
    size_t nodes_size{};

    [[nodiscard]] constexpr size_t CharToNodeIndex(uint8_t chr) const noexcept {
        return static_cast<size_t>(chr) - min_char;
    }
    [[nodiscard]] constexpr size_t CharToNodeIndex(char chr) const noexcept {
        return CharToNodeIndex(static_cast<uint8_t>(chr));
    }
};

};  // namespace trie_tools

namespace counting_tools {

struct MinMaxCharsType {
    uint32_t min_char;
    uint32_t max_char;
};

template <detail::CompileTimeStringLiteral FirstString,
          detail::CompileTimeStringLiteral... Strings>
consteval MinMaxCharsType FindMinMaxChars() noexcept {
    // std::min and std::max are not used in order not to
    // include additional header (<algorithm>) just for min/max.
    constexpr auto min_fn = [](uint32_t a, uint32_t b) constexpr noexcept {
        return a <= b ? a : b;
    };
    constexpr auto max_fn = [](uint32_t a, uint32_t b) constexpr noexcept {
        return a >= b ? a : b;
    };

    // Not FirstString.size() because it can be 0.
    uint32_t min_char    = 256;
    uint32_t max_char    = 0;
    constexpr size_t len = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        const uint32_t chr = static_cast<uint8_t>(FirstString.value[i]);
        min_char           = min_fn(min_char, chr);
        max_char           = max_fn(max_char, chr);
    }
    if constexpr (sizeof...(Strings) > 0) {
        constexpr MinMaxCharsType mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char                              = min_fn(min_char, mn_mx_chars.min_char);
        max_char                              = max_fn(max_char, mn_mx_chars.max_char);
    }

    if (min_char == 256) {
        // All string are empty.
        min_char = 0;
    }

    return {
        .min_char = min_char,
        .max_char = max_char,
    };
}

template <size_t AlphabetSize>
struct CountingNode final {
    std::array<uint32_t, AlphabetSize> edges{};
};

template <trie_tools::TrieParamsType TrieParams, detail::CompileTimeStringLiteral FirstString,
          detail::CompileTimeStringLiteral... Strings>
consteval size_t CountNodesImpl(
    std::vector<CountingNode<TrieParams.trie_alphabet_size>>& nodes) {
    size_t current_node_index = 0;
    constexpr size_t len      = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        size_t index           = TrieParams.CharToNodeIndex(FirstString.value[i]);
        size_t next_node_index = nodes[current_node_index].edges[index];
        if (next_node_index == 0) {
            size_t new_node_index = nodes.size();
            nodes.emplace_back();
            nodes[current_node_index].edges[index] = uint32_t(new_node_index);
            next_node_index                        = new_node_index;
        }
        current_node_index = next_node_index;
    }

    if constexpr (sizeof...(Strings) > 0) {
        return CountNodesImpl<TrieParams, Strings...>(nodes);
    } else {
        return nodes.size();
    }
}

template <trie_tools::TrieParamsType TrieParams, detail::CompileTimeStringLiteral... Strings>
consteval size_t CountNodes() {
    std::vector<CountingNode<TrieParams.trie_alphabet_size>> nodes(size_t(1));
    return CountNodesImpl<TrieParams, Strings...>(nodes);
}

template <detail::CompileTimeStringLiteral... Strings>
consteval trie_tools::TrieParamsType TrieParams() {
    constexpr MinMaxCharsType kMinMaxChars           = FindMinMaxChars<Strings...>();
    constexpr trie_tools::TrieParamsType kTrieParams = {
        .min_char = kMinMaxChars.min_char,
        .max_char = kMinMaxChars.max_char,
    };
    return {
        .min_char   = kTrieParams.min_char,
        .max_char   = kTrieParams.max_char,
        .nodes_size = CountNodes<kTrieParams, Strings...>(),
    };
}

}  // namespace counting_tools

namespace string_map_impl {

template <trie_tools::TrieParamsType TrieParams, std::array MappedValues,
          decltype(MappedValues)::value_type DefaultMapValue,
          detail::CompileTimeStringLiteral... Strings>
class StringMapImpl final {
    static_assert(0 < TrieParams.min_char && TrieParams.min_char <= TrieParams.max_char &&
                      TrieParams.max_char <= std::numeric_limits<uint8_t>::max(),
                  "Empty string was passed in StringMatch / StringMap");

public:
    using MappedType = decltype(MappedValues)::value_type;
    static_assert(std::is_nothrow_copy_assignable_v<MappedType> &&
                  std::is_trivially_copy_assignable_v<MappedType>);

    static constexpr MappedType kDefaultValue = DefaultMapValue;
    static constexpr char kMinChar            = static_cast<char>(TrieParams.min_char);
    static constexpr char kMaxChar            = static_cast<char>(TrieParams.max_char);

    consteval StringMapImpl() noexcept {
        AddPattern<0, Strings...>(kRootNodeIndex + 1);
    }

    [[nodiscard]] consteval MappedType operator()(std::nullptr_t) const noexcept = delete;
    [[nodiscard]] constexpr MappedType operator()(std::string_view str) const noexcept {
        return operator()(str.data());
    }
    [[nodiscard]] constexpr MappedType operator()(const std::string& str) const noexcept {
        return operator()(str.c_str());
    }
    [[nodiscard]] constexpr MappedType operator()(const char* str) const noexcept {
        if (str == nullptr) [[unlikely]] {
            return kDefaultValue;
        }

        size_t current_node_index = kRootNodeIndex;
        if (std::is_constant_evaluated()) {
            for (char c = 0; (c = *str) != '\0'; ++str) {
                size_t index = TrieParams.CharToNodeIndex(c);
                if (index >= kTrieAlphabetSize) {
                    return kDefaultValue;
                }

                size_t next_node_index = nodes_[current_node_index].edges[index];
                if (next_node_index != 0) {
                    current_node_index = next_node_index;
                } else {
                    return kDefaultValue;
                }
            }
        } else {
            const uint8_t* ustr = reinterpret_cast<const uint8_t*>(str);
            for (uint8_t c; (c = *ustr) != '\0'; ++ustr) {
                size_t index = TrieParams.CharToNodeIndex(c);
                if (index >= kTrieAlphabetSize) {
                    return kDefaultValue;
                }

                size_t next_node_index = nodes_[current_node_index].edges[index];
                if (next_node_index != 0) {
                    current_node_index = next_node_index;
                } else {
                    return kDefaultValue;
                }
            }
        }

        return nodes_[current_node_index].node_value;
    }

private:
    using NodeIndex = uint32_t;

    static constexpr NodeIndex kRootNodeIndex = TrieParams.kRootNodeIndex;
    static constexpr size_t kTrieAlphabetSize = TrieParams.trie_alphabet_size;
    static constexpr size_t kNodesSize        = TrieParams.nodes_size;

    struct TrieNodeImpl final {
        std::array<NodeIndex, kTrieAlphabetSize> edges{};
        MappedType node_value = DefaultMapValue;
    };
    std::array<TrieNodeImpl, kNodesSize> nodes_{};

    template <size_t CurrentPackIndex, detail::CompileTimeStringLiteral String,
              detail::CompileTimeStringLiteral... AddStrings>
    consteval void AddPattern(size_t first_free_node_index) noexcept {
        size_t current_node_index = 0;
        constexpr size_t len      = String.size();
        for (size_t i = 0; i < len; i++) {
            size_t symbol_index    = TrieParams.CharToNodeIndex(String.value[i]);
            size_t next_node_index = nodes_[current_node_index].edges[symbol_index];
            if (next_node_index == 0) {
                nodes_[current_node_index].edges[symbol_index] =
                    static_cast<NodeIndex>(first_free_node_index);
                next_node_index = first_free_node_index;
                first_free_node_index++;
            }
            current_node_index = next_node_index;
        }

        // Division by zero will occur in compile time if
        //  the same patterns are added to the trie.
        [[maybe_unused]] const auto duplicate_strings_check =
            0 / (nodes_[current_node_index].node_value == DefaultMapValue);
        static_assert(CurrentPackIndex < MappedValues.size(), "impl error");
        nodes_[current_node_index].node_value = MappedValues[CurrentPackIndex];
        if constexpr (sizeof...(AddStrings) > 0) {
            AddPattern<CurrentPackIndex + 1, AddStrings...>(first_free_node_index);
        }
    }
};

}  // namespace string_map_impl

template <size_t N>
consteval std::array<std::size_t, N> make_index_array() noexcept {
    std::array<std::size_t, N> index_array{};
#if defined(__cpp_lib_ranges_iota) && __cpp_lib_ranges_iota >= 202202L
    std::ranges::iota(index_array, 0);
#else
    std::iota(index_array.begin(), index_array.end(), 0);
#endif
    return index_array;
}

};  // namespace detail

template <std::array MappedValues, decltype(MappedValues)::value_type DefaultMapValue,
          detail::CompileTimeStringLiteral... Strings>
    requires(sizeof...(Strings) == MappedValues.size())
using StringMap = typename detail::string_map_impl::StringMapImpl<
    detail::counting_tools::TrieParams<Strings...>(), MappedValues, DefaultMapValue,
    Strings...>;

template <detail::CompileTimeStringLiteral... Strings>
using StringMatch =
    StringMap<detail::make_index_array<sizeof...(Strings)>(), sizeof...(Strings), Strings...>;
