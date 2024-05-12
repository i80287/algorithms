#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CompileTimeContainers.hpp"

namespace detail {

namespace trie_functions {

struct TrieParamsType final {
    uint32_t min_char;
    uint32_t max_char;
    size_t trie_alphabet_size = max_char - min_char + 1;

    static constexpr size_t kRootNodeIndex = 0;
};

template <TrieParamsType TrieParams>
constexpr size_t Uint8ToNodeIndex(uint8_t chr) noexcept {
    return static_cast<size_t>(chr) - TrieParams.min_char;
}

};  // namespace trie_functions

namespace counting_tools {

using trie_functions::TrieParamsType;

template <detail::CompileTimeStringLiteral FirstString,
          detail::CompileTimeStringLiteral... Strings>
consteval TrieParamsType FindMinMaxChars() noexcept {
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
        const TrieParamsType mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char                         = min_fn(min_char, mn_mx_chars.min_char);
        max_char                         = max_fn(max_char, mn_mx_chars.max_char);
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

template <TrieParamsType TrieParams>
struct CountingNode final {
    std::array<uint32_t, TrieParams.trie_alphabet_size> edges{};
};

template <TrieParamsType TrieParams, detail::CompileTimeStringLiteral FirstString,
          detail::CompileTimeStringLiteral... Strings>
consteval size_t CountNodesImpl(std::vector<CountingNode<TrieParams>>& nodes) {
    size_t current_node_index = 0;
    constexpr size_t len      = FirstString.size();
    for (size_t i = 0; i < len; i++) {
        uint32_t symbol        = static_cast<uint8_t>(FirstString.value[i]);
        size_t index           = symbol - TrieParams.min_char;
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

template <TrieParamsType TrieParams, detail::CompileTimeStringLiteral... Strings>
consteval size_t CountNodes() {
    std::vector<CountingNode<TrieParams>> nodes(size_t(1));
    return CountNodesImpl<TrieParams, Strings...>(nodes);
}

template <detail::CompileTimeStringLiteral... Strings>
inline constexpr TrieParamsType TrieParams = FindMinMaxChars<Strings...>();

template <detail::CompileTimeStringLiteral... Strings>
inline constexpr size_t kNodesCount = CountNodes<TrieParams<Strings...>, Strings...>();

}  // namespace counting_tools

namespace string_map_impl {

template <counting_tools::TrieParamsType TrieParams, size_t NodesCount, class MappedType,
          class MapValuesContainer, MappedType DefaultMapValue,
          detail::CompileTimeStringLiteral... Strings>
class StringMapImpl;

template <counting_tools::TrieParamsType TrieParams, size_t NodesCount,
          detail::CompileTimeStringLiteral... Strings, class MappedType,
          MappedType DefaultMapValue, MappedType... Values>
class StringMapImpl<TrieParams, NodesCount, MappedType, TypedValuesList<MappedType, Values...>,
                    DefaultMapValue, Strings...> {
    static_assert(0 < TrieParams.min_char && TrieParams.min_char <= TrieParams.max_char &&
                      TrieParams.max_char <= std::numeric_limits<uint8_t>::max(),
                  "Empty strings were provided in StringMatch");

    static constexpr size_t kRootNodeIndex    = TrieParams.kRootNodeIndex;
    static constexpr size_t kTrieAlphabetSize = TrieParams.trie_alphabet_size;
    static constexpr size_t kNodesSize        = NodesCount;

    struct TrieNodeImpl final {
        std::array<uint32_t, kTrieAlphabetSize> edges{};
        MappedType node_value = DefaultMapValue;
    };
    std::array<TrieNodeImpl, kNodesSize> nodes_{};

    template <size_t CurrentPackIndex, detail::CompileTimeStringLiteral String,
              detail::CompileTimeStringLiteral... AddStrings>
    consteval void AddPattern(size_t first_free_node_index) noexcept {
        size_t current_node_index = 0;
        constexpr size_t len      = String.size();
        for (size_t i = 0; i < len; i++) {
            size_t index = detail::trie_functions::Uint8ToNodeIndex<TrieParams>(
                static_cast<uint8_t>(String.value[i]));
            size_t child_index = nodes_[current_node_index].edges[index];
            if (child_index != 0) {
                current_node_index = child_index;
            } else {
                nodes_[current_node_index].edges[index] =
                    static_cast<uint32_t>(first_free_node_index);
                current_node_index = first_free_node_index;
                first_free_node_index++;
            }
        }

        // Division by zero will occur in compile time if
        //  the same patterns are added to the trie.
        [[maybe_unused]] const auto duplicate_strings_check =
            0 / (nodes_[current_node_index].node_value == DefaultMapValue);
        nodes_[current_node_index].node_value =
            PackElementAtIndex<CurrentPackIndex, MappedType, Values...>();
        if constexpr (CurrentPackIndex + 1 < sizeof...(Values)) {
            static_assert(sizeof...(AddStrings) > 0, "impl error");
            AddPattern<CurrentPackIndex + 1, AddStrings...>(first_free_node_index);
        }
    }

public:
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

    [[nodiscard]] constexpr MappedType operator()(const char* str) const noexcept {
        if (str == nullptr) [[unlikely]] {
            return kDefaultValue;
        }

        size_t current_node_index = kRootNodeIndex;
        if (std::is_constant_evaluated()) {
            for (char c = 0; (c = *str) != '\0'; ++str) {
                size_t index = detail::trie_functions::Uint8ToNodeIndex<TrieParams>(
                    static_cast<uint8_t>(c));
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
                size_t index = detail::trie_functions::Uint8ToNodeIndex<TrieParams>(c);
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
};

}  // namespace string_map_impl

template <class StringKeysContainer, class MappedType, class MappedValuesContainer,
          MappedType DefaultMapValue>
struct StringMapHelper;

template <detail::CompileTimeStringLiteral... Strings, class MappedType,
          MappedType DefaultMapValue, MappedType... Values>
    requires(sizeof...(Strings) == sizeof...(Values)) &&
            std::is_trivially_default_constructible_v<MappedType> &&
            std::is_nothrow_default_constructible_v<MappedType> &&
            std::is_nothrow_copy_assignable_v<MappedType> &&
            std::is_trivially_copy_assignable_v<MappedType>
struct StringMapHelper<StringsList<Strings...>, MappedType,
                       TypedValuesList<MappedType, Values...>, DefaultMapValue> {
    using Type = detail::string_map_impl::StringMapImpl<
        detail::counting_tools::TrieParams<Strings...>,
        detail::counting_tools::kNodesCount<Strings...>, MappedType,
        TypedValuesList<MappedType, Values...>, DefaultMapValue, Strings...>;
};

};  // namespace detail

template <class StringKeysContainer, class MappedValuesContainer,
          typename MappedValuesContainer::Type DefaultMapValue =
              static_cast<MappedValuesContainer::Type>(MappedValuesContainer::size())>
using StringMap =
    detail::StringMapHelper<StringKeysContainer, typename MappedValuesContainer::Type,
                            MappedValuesContainer, DefaultMapValue>::Type;

template <detail::CompileTimeStringLiteral... Strings>
using StringMatch = StringMap<
    StringsList<Strings...>,
    IntegerSequenceToTypedValuesList<std::make_integer_sequence<uint32_t, sizeof...(Strings)>>,
    sizeof...(Strings)>;
