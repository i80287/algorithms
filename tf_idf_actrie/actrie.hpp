#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

namespace actrie {

template <std::uint8_t AlphabetStart = 'A', std::uint8_t AlphabetEnd = 'z',
          bool IsCaseInsensetive = false>
class ACTrieBuilder;

template <std::uint8_t AlphabetStart = 'A', std::uint8_t AlphabetEnd = 'z',
          bool IsCaseInsensetive = false>
class [[nodiscard]] ACTrie final {
public:
    friend class ACTrieBuilder<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;

    using size_type          = std::size_t;
    using StoredNodeIndex    = std::uint32_t;
    using StoredPatternSize  = std::uint32_t;
    using StoredPatternIndex = std::uint32_t;

private:
    using Symbol = unsigned char;

    static constexpr Symbol kAlphabetStart     = AlphabetStart;
    static constexpr Symbol kAlphabetEnd       = AlphabetEnd;
    static constexpr Symbol kIsCaseInsensetive = IsCaseInsensetive;

    static_assert('\0' < kAlphabetStart && kAlphabetStart < kAlphabetEnd &&
                  kAlphabetEnd < std::numeric_limits<char>::max());
    static constexpr Symbol kAlphabetLength = kAlphabetEnd - kAlphabetStart + 1;

    static constexpr StoredNodeIndex kNullNodeIndex        = 0;
    static constexpr StoredNodeIndex kFakePrerootNodeIndex = kNullNodeIndex + 1;
    static constexpr StoredNodeIndex kRootNodeIndex        = kFakePrerootNodeIndex + 1;

    using EdgesArray = std::array<StoredNodeIndex, kAlphabetLength>;

    static constexpr EdgesArray CreateEdgesArray() noexcept {
        EdgesArray edges{};
        edges.fill(kNullNodeIndex);
        return edges;
    }

    struct Node {
        static constexpr StoredPatternIndex kMissingWordIndex =
            std::numeric_limits<StoredPatternIndex>::max();

        EdgesArray edges                       = CreateEdgesArray();
        StoredNodeIndex suffix_link            = kNullNodeIndex;
        StoredNodeIndex compressed_suffix_link = kNullNodeIndex;
        StoredPatternIndex pattern_index       = kMissingWordIndex;

        [[nodiscard]] constexpr StoredNodeIndex operator[](size_type index) const noexcept {
            return edges[index];
        }
        [[nodiscard]] constexpr StoredNodeIndex& operator[](size_type index) noexcept {
            return edges[index];
        }
        [[nodiscard]] constexpr bool IsTerminal() const noexcept {
            return pattern_index != kMissingWordIndex;
        }
    };

public:
    [[nodiscard]] constexpr bool ContainsPattern(std::string_view pattern) const noexcept {
        if (std::is_constant_evaluated()) {
            return ContainsPatternImpl(pattern.begin(), pattern.end(), nodes_);
        } else {
            const unsigned char* pattern_iter_begin =
                reinterpret_cast<const unsigned char*>(pattern.data());
            const unsigned char* pattern_iter_end = pattern_iter_begin + pattern.size();
            return ContainsPatternImpl(pattern_iter_begin, pattern_iter_end, nodes_);
        }
    }

    template <typename FindCallback>
        requires requires(FindCallback func, std::string_view found_word,
                          size_type start_index_in_original_text) {
            func(found_word, start_index_in_original_text);
        }
    constexpr void FindAllSubstringsInText(std::string_view text,
                                           FindCallback find_callback) const {
        if constexpr (std::is_convertible_v<decltype(find_callback), bool>) {
            if (!std::is_constant_evaluated()) {
                assert(find_callback);
            }
        }

        auto current_node_index = kRootNodeIndex;
        size_type i             = 0;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter, ++i) {
            size_type symbol_index = SymbolToIndex(*iter);
            if (symbol_index >= kAlphabetLength) {
                current_node_index = kRootNodeIndex;
                continue;
            }

            current_node_index = nodes_[current_node_index][symbol_index];
            if (!std::is_constant_evaluated()) {
                assert(current_node_index != kNullNodeIndex);
            }
            const Node& node = nodes_[current_node_index];
            if (node.IsTerminal()) {
                size_type pattern_index = node.pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }
                size_type word_length = patterns_lengths_[pattern_index];
                size_type l           = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }

            for (auto terminal_node_index = node.compressed_suffix_link;
                 terminal_node_index != kRootNodeIndex;
                 terminal_node_index = nodes_[terminal_node_index].compressed_suffix_link) {
                if (!std::is_constant_evaluated()) {
                    assert(terminal_node_index != kNullNodeIndex);
                    assert(nodes_[terminal_node_index].IsTerminal());
                }
                size_type pattern_index = nodes_[terminal_node_index].pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }
                size_type word_length = patterns_lengths_[pattern_index];
                size_type l           = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }
        }
    }

    template <bool IsExactWordsMatching = true, bool CountEmptyLines = true,
              Symbol LinesDelimeter = '\n', typename QueryWordCallback, typename NewLineCallback>
        requires requires(QueryWordCallback func, size_type line_number,
                          StoredPatternIndex query_word_index) {
            func(line_number, query_word_index);
        } && requires(NewLineCallback func, size_type line_number, size_type words_on_current_line,
                      size_type line_start_index, size_type line_end_index) {
            func(line_number, words_on_current_line, line_start_index, line_end_index);
        }
    [[nodiscard]]
    constexpr size_type FindAllSubstringsInTextAndCountLines(std::string_view text,
                                                             QueryWordCallback find_callback,
                                                             NewLineCallback line_callback) const {
        static_assert(!(kAlphabetStart <= LinesDelimeter && LinesDelimeter <= kAlphabetEnd) &&
                          SymbolToIndex(LinesDelimeter) >= kAlphabetLength,
                      "Lines delimeter can\'t be in the alphabet");
        if constexpr (std::is_pointer_v<decltype(find_callback)>) {
            if (!std::is_constant_evaluated()) {
                assert(find_callback != nullptr);
            }
        }
        if constexpr (std::is_pointer_v<decltype(line_callback)>) {
            if (!std::is_constant_evaluated()) {
                assert(line_callback != nullptr);
            }
        }

        auto current_node_index         = kRootNodeIndex;
        size_type line_start_index      = 0;
        size_type i                     = 0;
        size_type current_line          = 1;
        size_type words_on_current_line = 0;
        size_type lines_count           = 0;
        bool prev_symbol_in_alphabet    = false;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter, ++i) {
            const Symbol symbol          = CharToSymbol(*iter);
            const size_type symbol_index = SymbolToIndex(symbol);
            if (symbol_index >= kAlphabetLength) {
                current_node_index = kRootNodeIndex;
                words_on_current_line += prev_symbol_in_alphabet;
                if (symbol == LinesDelimeter) {
                    assert(line_start_index <= i);
                    if (CountEmptyLines || line_start_index != i) {
                        line_callback(current_line, words_on_current_line, line_start_index, i);
                        lines_count++;
                    }
                    current_line++;
                    line_start_index      = i + 1;
                    words_on_current_line = 0;
                }

                prev_symbol_in_alphabet = false;
                continue;
            }

            current_node_index = nodes_[current_node_index][symbol_index];
            if (!std::is_constant_evaluated()) {
                assert(current_node_index != kNullNodeIndex);
            }
            const Node& node = nodes_[current_node_index];
            if (node.IsTerminal()) {
                auto pattern_index = node.pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }

                if constexpr (IsExactWordsMatching) {
                    size_type word_length      = patterns_lengths_[pattern_index];
                    size_type word_begin_index = i + 1 - word_length;
                    bool prev_symbol_and_next_symbol_not_in_the_alphabet =
                        (word_begin_index == 0 || !IsInAlphabet(text[word_begin_index - 1])) &&
                        (i + 1 == text.size() || !IsInAlphabet(text[i + 1]));
                    if (prev_symbol_and_next_symbol_not_in_the_alphabet) {
                        find_callback(current_line, pattern_index);
                    }
                } else {
                    find_callback(current_line, pattern_index);
                }
            }

            // Jump up through compressed suffix links
            for (uint32_t terminal_node_index = node.compressed_suffix_link;
                 terminal_node_index != kRootNodeIndex;
                 terminal_node_index = nodes_[terminal_node_index].compressed_suffix_link) {
                if (!std::is_constant_evaluated()) {
                    assert(terminal_node_index != kNullNodeIndex &&
                           nodes_[terminal_node_index].IsTerminal());
                }
                size_type pattern_index = nodes_[terminal_node_index].pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }

                if constexpr (IsExactWordsMatching) {
                    size_type word_length = patterns_lengths_[pattern_index];
                    size_type l           = i + 1 - word_length;
                    if ((l == 0 || !IsInAlphabet(text[l - 1])) &&
                        (i + 1 == text.size() || !IsInAlphabet(text[i + 1]))) {
                        find_callback(current_line, pattern_index);
                    }
                } else {
                    find_callback(current_line, pattern_index);
                }
            }

            prev_symbol_in_alphabet = true;
        }

        if (!text.empty() && CharToSymbol(text.back()) != LinesDelimeter) {
            lines_count++;
        }

        return lines_count;
    }

    constexpr size_type PatternsSize() const noexcept {
        return patterns_lengths_.size();
    }

private:
    constexpr ACTrie(std::vector<Node>&& nodes,
                     std::vector<StoredPatternSize>&& words_lengths) noexcept
        : nodes_(std::move(nodes)), patterns_lengths_(std::move(words_lengths)) {}

    template <class PatternIterator>
    [[nodiscard]]
    static constexpr bool ContainsPatternImpl(PatternIterator pattern_iter_begin,
                                              PatternIterator pattern_iter_end,
                                              const std::vector<Node>& nodes) noexcept {
        size_type current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end; ++pattern_iter_begin) {
            size_type index = SymbolToIndex(*pattern_iter_begin);
            if (index >= kAlphabetLength) {
                return false;
            }

            size_type next_node_index = nodes[current_node_index][index];
            if (next_node_index != kNullNodeIndex) {
                current_node_index = next_node_index;
            } else {
                return false;
            }
        }

        return nodes[current_node_index].IsTerminal();
    }

    [[nodiscard]] static constexpr int32_t ToLowerImpl(int32_t c) noexcept {
        static_assert('a' - 'A' == (1 << 5));
        return c | (IsUpperImpl(c) * ('a' - 'A'));
    }
    [[nodiscard]] static constexpr bool IsUpperImpl(int32_t c) noexcept {
        return static_cast<std::uint32_t>(c) - 'A' <= 'Z' - 'A';
    }
    [[nodiscard]] static constexpr bool IsInAlphabet(Symbol symbol) noexcept {
        return static_cast<std::uint32_t>(symbol) - kAlphabetStart <= kAlphabetEnd - kAlphabetStart;
    }
    [[nodiscard]] static constexpr bool IsInAlphabet(char symbol) noexcept {
        return IsInAlphabet(CharToSymbol(symbol));
    }
    [[nodiscard]] static constexpr size_type SymbolToIndex(Symbol symbol) noexcept {
        std::int32_t symbol_as_int = symbol;
        if constexpr (kIsCaseInsensetive) {
            // We don't use std::tolower because we know that all
            //  chars are < 128. On the other side, std::tolower makes
            //  text finding runs almost 1.5x times slower because of
            //  the locale handling.
            symbol_as_int = ToLowerImpl(symbol_as_int);
        }

        return static_cast<size_type>(static_cast<std::uint32_t>(symbol_as_int)) - kAlphabetStart;
    }
    [[nodiscard]] static constexpr size_type SymbolToIndex(char chr) noexcept {
        return SymbolToIndex(CharToSymbol(chr));
    }
    [[nodiscard]] static constexpr Symbol CharToSymbol(char chr) noexcept {
        static_assert(std::is_same_v<Symbol, unsigned char>);
        return static_cast<Symbol>(chr);
    }

    std::vector<Node> nodes_;
    std::vector<StoredPatternSize> patterns_lengths_;
};

template <std::uint8_t AlphabetStart, std::uint8_t AlphabetEnd, bool IsCaseInsensetive>
class [[nodiscard]] ACTrieBuilder final {
public:
    using ACTrieType         = ACTrie<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;
    using size_type          = typename ACTrieType::size_type;
    using StoredNodeIndex    = typename ACTrieType::StoredNodeIndex;
    using StoredPatternSize  = typename ACTrieType::StoredPatternSize;
    using StoredPatternIndex = typename ACTrieType::StoredPatternIndex;

private:
    using Node = typename ACTrieType::Node;

    static constexpr auto kAlphabetStart        = ACTrieType::kAlphabetStart;
    static constexpr auto kAlphabetEnd          = ACTrieType::kAlphabetEnd;
    static constexpr auto kAlphabetLength       = ACTrieType::kAlphabetLength;
    static constexpr auto kIsCaseInsensetive    = ACTrieType::kIsCaseInsensetive;
    static constexpr auto kNullNodeIndex        = ACTrieType::kNullNodeIndex;
    static constexpr auto kRootNodeIndex        = ACTrieType::kRootNodeIndex;
    static constexpr auto kFakePrerootNodeIndex = ACTrieType::kFakePrerootNodeIndex;

public:
    constexpr ACTrieBuilder() {
        constexpr size_type kDefaultNodesSize =
            std::max({kNullNodeIndex, kFakePrerootNodeIndex, kRootNodeIndex}) + 1;
        constexpr size_type kDefaultNodesCapacity = std::max(kDefaultNodesSize, size_type(32));

        nodes_.reserve(kDefaultNodesCapacity);
        nodes_.resize(kDefaultNodesSize);
    }

    constexpr ACTrieBuilder(size_type patterns_capacity) : ACTrieBuilder() {
        patterns_lengths_.reserve(patterns_capacity);
    }

    [[nodiscard]] constexpr bool AddPattern(std::string_view pattern) {
        if (std::is_constant_evaluated()) {
            return AddPatternImpl(pattern.begin(), pattern.end(), nodes_, patterns_lengths_);
        } else {
            const unsigned char* pattern_iter_begin =
                reinterpret_cast<const unsigned char*>(pattern.data());
            const unsigned char* pattern_iter_end = pattern_iter_begin + pattern.size();
            return AddPatternImpl(pattern_iter_begin, pattern_iter_end, nodes_, patterns_lengths_);
        }
    }

    [[nodiscard]] constexpr bool ContainsPattern(std::string_view pattern) const noexcept {
        if (std::is_constant_evaluated()) {
            return ContainsPatternImpl(pattern.begin(), pattern.end(), nodes_);
        } else {
            const unsigned char* pattern_iter_begin =
                reinterpret_cast<const unsigned char*>(pattern.data());
            const unsigned char* pattern_iter_end = pattern_iter_begin + pattern.size();
            return ContainsPatternImpl(pattern_iter_begin, pattern_iter_end, nodes_);
        }
    }

    [[nodiscard]] constexpr ACTrieType Build() && {
        nodes_[kRootNodeIndex].suffix_link            = kFakePrerootNodeIndex;
        nodes_[kRootNodeIndex].compressed_suffix_link = kRootNodeIndex;
        nodes_[kFakePrerootNodeIndex].edges.fill(kRootNodeIndex);
        // We use std::vector instead of std::queue
        //  in order to make this method constexpr.
        std::vector<size_type> bfs_queue(nodes_.size());
        size_type queue_head    = 0;
        size_type queue_tail    = 0;
        bfs_queue[queue_tail++] = kRootNodeIndex;
        do {
            size_type node_index = bfs_queue[queue_head++];
            ComputeLinksForNodeChildren(node_index, nodes_, bfs_queue, queue_tail);
        } while (queue_head < queue_tail);
        return ACTrieType(std::move(nodes_), std::move(patterns_lengths_));
    }

private:
    static constexpr void ComputeLinksForNodeChildren(size_type node_index,
                                                      std::vector<Node>& nodes,
                                                      std::vector<size_type>& bfs_queue,
                                                      size_type& queue_tail) noexcept {
        Node& node = nodes[node_index];
        for (size_type symbol_index = 0; symbol_index < kAlphabetLength; symbol_index++) {
            const StoredNodeIndex child_link_v_index = nodes[node.suffix_link][symbol_index];
            const size_type child_index              = node[symbol_index];
            if (child_index != kNullNodeIndex) {
                nodes[child_index].suffix_link = child_link_v_index;
                const StoredNodeIndex child_comp_sl =
                    nodes[child_link_v_index].compressed_suffix_link;
                const bool is_terminal_or_root =
                    nodes[child_link_v_index].IsTerminal() || child_link_v_index == kRootNodeIndex;
                nodes[child_index].compressed_suffix_link =
                    is_terminal_or_root ? child_link_v_index : child_comp_sl;
                bfs_queue[queue_tail++] = child_index;
            } else {
                node[symbol_index] = child_link_v_index;
            }
        }
    }

    template <class PatternIterator>
    [[nodiscard]]
    static constexpr bool AddPatternImpl(PatternIterator pattern_iter_begin,
                                         PatternIterator pattern_iter_end, std::vector<Node>& nodes,
                                         std::vector<StoredPatternSize>& words_lengths) noexcept {
        const auto pattern_size = static_cast<size_type>(pattern_iter_end - pattern_iter_begin);
        size_type current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end; ++pattern_iter_begin) {
            auto symbol_index = ACTrieType::SymbolToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }
            size_type next_node_index = nodes[current_node_index][symbol_index];
            if (next_node_index != kNullNodeIndex) {
                current_node_index = next_node_index;
            } else {
                break;
            }
        }

        const auto lasted_max_length =
            static_cast<size_type>(pattern_iter_end - pattern_iter_begin);
        nodes.reserve(nodes.size() + lasted_max_length);
        for (size_type new_node_index = nodes.size(); pattern_iter_begin != pattern_iter_end;
             ++pattern_iter_begin) {
            auto symbol_index = ACTrieType::SymbolToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }

            nodes.emplace_back();
            nodes[current_node_index][symbol_index] = static_cast<StoredNodeIndex>(new_node_index);
            current_node_index                      = new_node_index++;
        }

        StoredPatternIndex pattern_index = static_cast<StoredPatternIndex>(words_lengths.size());
        nodes[current_node_index].pattern_index = pattern_index;
        words_lengths.push_back(static_cast<StoredPatternSize>(pattern_size));
        return true;
    }

    std::vector<Node> nodes_;
    std::vector<StoredPatternSize> patterns_lengths_;
};

}  // namespace actrie
