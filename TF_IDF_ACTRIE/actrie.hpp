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

template <uint8_t kAlphabetStart = 'A', uint8_t kAlphabetEnd = 'z',
          bool kIsCaseInsensetive = false>
class ACTrieBuilder {
    static_assert('\0' < kAlphabetStart && kAlphabetStart < kAlphabetEnd &&
                  kAlphabetEnd < std::numeric_limits<char>::max());
    static constexpr uint8_t kAlphabetLength =
        kAlphabetEnd - kAlphabetStart + 1;

    using uint8_t            = std::uint8_t;
    using uint32_t           = std::uint32_t;
    using size_t             = std::size_t;
    using StoredNodeIndex    = uint32_t;
    using StoredPatternSize  = uint32_t;
    using StoredPatternIndex = uint32_t;

    static constexpr StoredNodeIndex kNullNodeIndex = 0;
    static constexpr StoredNodeIndex kFakePrerootNodeIndex =
        kNullNodeIndex + 1;
    static constexpr StoredNodeIndex kRootNodeIndex =
        kFakePrerootNodeIndex + 1;
    static constexpr size_t kDefaultNodesCount =
        kRootNodeIndex + 1;  // null node; fake preroot node; root node
    static constexpr size_t kDefaultNodesCapacity = 32;

    struct Node {
        static constexpr StoredPatternIndex kMissingWordIndex =
            std::numeric_limits<StoredPatternIndex>::max();

        std::array<StoredNodeIndex, kAlphabetLength> edges{kNullNodeIndex};
        StoredNodeIndex suffix_link            = kNullNodeIndex;
        StoredNodeIndex compressed_suffix_link = kNullNodeIndex;
        StoredPatternIndex pattern_index       = kMissingWordIndex;

        constexpr StoredNodeIndex operator[](size_t index) const noexcept {
            return edges[index];
        }

        constexpr StoredNodeIndex& operator[](size_t index) noexcept {
            return edges[index];
        }

        constexpr bool IsTerminal() const noexcept {
            return pattern_index != kMissingWordIndex;
        }
    };

public:
    constexpr ACTrieBuilder() {
        nodes_.reserve(kDefaultNodesCapacity);
        nodes_.resize(kDefaultNodesCount);
    }

    constexpr ACTrieBuilder(size_t patterns_capacity) : ACTrieBuilder() {
        patterns_lengths_.reserve(patterns_capacity);
    }

    constexpr bool AddPattern(std::string_view pattern) {
        if (std::is_constant_evaluated()) {
            return AddPatternImpl(pattern.begin(), pattern.end(), nodes_,
                                  patterns_lengths_);
        } else {
            const unsigned char* pattern_iter_begin =
                reinterpret_cast<const unsigned char*>(pattern.data());
            const unsigned char* pattern_iter_end =
                pattern_iter_begin + pattern.size();
            return AddPatternImpl(pattern_iter_begin, pattern_iter_end,
                                  nodes_, patterns_lengths_);
        }
    }

    constexpr bool ContainsPattern(
        std::string_view pattern) const noexcept {
        if (std::is_constant_evaluated()) {
            return ContainsPatternImpl(pattern.begin(), pattern.end(),
                                       nodes_);
        } else {
            const unsigned char* pattern_iter_begin =
                reinterpret_cast<const unsigned char*>(pattern.data());
            const unsigned char* pattern_iter_end =
                pattern_iter_begin + pattern.size();
            return ContainsPatternImpl(pattern_iter_begin,
                                       pattern_iter_end, nodes_);
        }
    }

    friend class ACTrie;

    class ACTrie {
    public:
        constexpr bool ContainsPattern(
            std::string_view pattern) const noexcept {
            if (std::is_constant_evaluated()) {
                return ContainsPatternImpl(pattern.begin(), pattern.end(),
                                           nodes_);
            } else {
                const unsigned char* pattern_iter_begin =
                    reinterpret_cast<const unsigned char*>(pattern.data());
                const unsigned char* pattern_iter_end =
                    pattern_iter_begin + pattern.size();
                return ContainsPatternImpl(pattern_iter_begin,
                                           pattern_iter_end, nodes_);
            }
        }

        template <typename FindCallback>
            requires requires(FindCallback func,
                              std::string_view found_word,
                              size_t start_index_in_original_text) {
                func(found_word, start_index_in_original_text);
            }
        constexpr void FindAllSubstringsInText(
            std::string_view text, FindCallback find_callback) const {
            if constexpr (std::is_pointer_v<decltype(find_callback)>) {
                if (!std::is_constant_evaluated()) {
                    assert(find_callback != nullptr);
                }
            }

            uint32_t current_node_index = kRootNodeIndex;
            size_t i                    = 0;
            for (auto iter = text.begin(), end = text.end(); iter != end;
                 ++iter, ++i) {
                size_t symbol_index = SymbolToIndex(*iter);
                if (symbol_index >= kAlphabetLength) {
                    current_node_index = kRootNodeIndex;
                    continue;
                }

                current_node_index =
                    nodes_[current_node_index][symbol_index];
                if (!std::is_constant_evaluated()) {
                    assert(current_node_index != kNullNodeIndex);
                }
                const Node& node = nodes_[current_node_index];
                if (node.IsTerminal()) {
                    size_t pattern_index = node.pattern_index;
                    if (!std::is_constant_evaluated()) {
                        assert(pattern_index < patterns_lengths_.size());
                    }
                    size_t word_length = patterns_lengths_[pattern_index];
                    size_t l           = i + 1 - word_length;
                    find_callback(text.substr(l, word_length), l);
                }

                for (uint32_t terminal_node_index =
                         node.compressed_suffix_link;
                     terminal_node_index != kRootNodeIndex;
                     terminal_node_index = nodes_[terminal_node_index]
                                               .compressed_suffix_link) {
                    if (!std::is_constant_evaluated()) {
                        assert(terminal_node_index != kNullNodeIndex);
                        assert(nodes_[terminal_node_index].IsTerminal());
                    }
                    size_t pattern_index =
                        nodes_[terminal_node_index].pattern_index;
                    if (!std::is_constant_evaluated()) {
                        assert(pattern_index < patterns_lengths_.size());
                    }
                    size_t word_length = patterns_lengths_[pattern_index];
                    size_t l           = i + 1 - word_length;
                    find_callback(text.substr(l, word_length), l);
                }
            }
        }

        template <bool IsExactWordsMatching = true,
                  uint8_t LinesDelimeter    = '\n',
                  typename QueryWordCallback, typename NewLineCallback>
            requires requires(QueryWordCallback func, size_t line_number,
                              size_t query_word_index) {
                func(line_number, query_word_index);
            } && requires(NewLineCallback func, size_t line_number,
                          size_t words_on_current_line,
                          size_t line_start_index, size_t line_end_index) {
                func(line_number, words_on_current_line, line_start_index,
                     line_end_index);
            }
        constexpr size_t FindAllSubstringsInTextAndCountLines(
            std::string_view text, QueryWordCallback find_callback,
            NewLineCallback line_callback) const {
            static_assert(!(kAlphabetStart <= LinesDelimeter &&
                            LinesDelimeter <= kAlphabetEnd));
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

            uint32_t current_node_index  = kRootNodeIndex;
            size_t line_start_index      = 0;
            size_t i                     = 0;
            size_t current_line          = 1;
            size_t words_on_current_line = 0;
            bool prev_symbol_in_alphabet = false;
            for (auto iter = text.begin(), end = text.end(); iter != end;
                 ++iter, ++i) {
                uint8_t symbol      = static_cast<uint8_t>(*iter);
                size_t symbol_index = SymbolToIndex(symbol);
                if (symbol_index >= kAlphabetLength) {
                    current_node_index = kRootNodeIndex;
                    words_on_current_line += prev_symbol_in_alphabet;
                    if (symbol == LinesDelimeter) {
                        if (line_start_index != i) {
                            line_callback(current_line,
                                          words_on_current_line,
                                          line_start_index, i);
                        }
                        line_start_index      = i + 1;
                        words_on_current_line = 0;
                        current_line++;
                    }

                    prev_symbol_in_alphabet = false;
                    continue;
                }

                current_node_index =
                    nodes_[current_node_index][symbol_index];
                if (!std::is_constant_evaluated()) {
                    assert(current_node_index != kNullNodeIndex);
                }
                const Node& node = nodes_[current_node_index];
                if (node.IsTerminal()) {
                    uint32_t pattern_index = node.pattern_index;
                    if (!std::is_constant_evaluated()) {
                        assert(pattern_index < patterns_lengths_.size());
                    }

                    if constexpr (IsExactWordsMatching) {
                        size_t word_length =
                            patterns_lengths_[pattern_index];
                        size_t l = i + 1 - word_length;
                        if ((l == 0 || !IsInAlphabet(text[l - 1])) &&
                            (i + 1 == text.size() ||
                             !IsInAlphabet(text[i + 1]))) {
                            find_callback(current_line, pattern_index);
                        }
                    } else {
                        find_callback(current_line, pattern_index);
                    }
                }

                // Jump up through compressed suffix links
                for (uint32_t terminal_node_index =
                         node.compressed_suffix_link;
                     terminal_node_index != kRootNodeIndex;
                     terminal_node_index = nodes_[terminal_node_index]
                                               .compressed_suffix_link) {
                    if (!std::is_constant_evaluated()) {
                        assert(terminal_node_index != kNullNodeIndex &&
                               nodes_[terminal_node_index].IsTerminal());
                    }
                    size_t pattern_index =
                        nodes_[terminal_node_index].pattern_index;
                    if (!std::is_constant_evaluated()) {
                        assert(pattern_index < patterns_lengths_.size());
                    }

                    if constexpr (IsExactWordsMatching) {
                        size_t word_length =
                            patterns_lengths_[pattern_index];
                        size_t l = i + 1 - word_length;
                        if ((l == 0 || !IsInAlphabet(text[l - 1])) &&
                            (i + 1 == text.size() ||
                             !IsInAlphabet(text[i + 1]))) {
                            find_callback(current_line, pattern_index);
                        }
                    } else {
                        find_callback(current_line, pattern_index);
                    }
                }

                prev_symbol_in_alphabet = true;
            }

            return current_line;
        }

        constexpr size_t PatternsSize() const noexcept {
            return patterns_lengths_.size();
        }

    private:
        using Builder = ACTrieBuilder<kAlphabetStart, kAlphabetEnd,
                                      kIsCaseInsensetive>;
        friend Builder;
        constexpr ACTrie(std::vector<Builder::Node>&& nodes,
                         std::vector<Builder::StoredPatternSize>&&
                             words_lengths) noexcept
            : nodes_(std::move(nodes)),
              patterns_lengths_(std::move(words_lengths)) {}

        std::vector<Builder::Node> nodes_;
        std::vector<Builder::StoredPatternSize> patterns_lengths_;
    };

    constexpr ACTrie Build() && {
        nodes_[kRootNodeIndex].suffix_link = kFakePrerootNodeIndex;
        nodes_[kRootNodeIndex].compressed_suffix_link = kRootNodeIndex;
        nodes_[kFakePrerootNodeIndex].edges.fill(kRootNodeIndex);
        // We use std::vector instead of std::queue
        //  in order to make this method constexpr.
        std::vector<size_t> bfs_queue(nodes_.size());
        size_t queue_head       = 0;
        size_t queue_tail       = 0;
        bfs_queue[queue_tail++] = kRootNodeIndex;
        do {
            size_t node_index = bfs_queue[queue_head++];
            ComputeLinksForNodeChildren(node_index, nodes_, bfs_queue,
                                        queue_tail);
        } while (queue_head < queue_tail);
        return ACTrie(std::move(nodes_), std::move(patterns_lengths_));
    }

private:
    static constexpr void ComputeLinksForNodeChildren(
        size_t node_index, std::vector<Node>& nodes,
        std::vector<size_t>& bfs_queue, size_t& queue_tail) noexcept {
        Node& node = nodes[node_index];
        for (size_t symbol_index = 0; symbol_index < kAlphabetLength;
             symbol_index++) {
            const StoredNodeIndex child_link_v_index =
                nodes[node.suffix_link][symbol_index];
            const size_t child_index = node[symbol_index];
            if (child_index != kNullNodeIndex) {
                nodes[child_index].suffix_link = child_link_v_index;
                const StoredNodeIndex child_comp_sl =
                    nodes[child_link_v_index].compressed_suffix_link;
                const bool is_terminal_or_root =
                    nodes[child_link_v_index].IsTerminal() ||
                    child_link_v_index == kRootNodeIndex;
                nodes[child_index].compressed_suffix_link =
                    is_terminal_or_root ? child_link_v_index
                                        : child_comp_sl;
                bfs_queue[queue_tail++] = child_index;
            } else {
                node[symbol_index] = child_link_v_index;
            }
        }
    }

    static constexpr int32_t ToLowerImpl(int32_t c) noexcept {
        static_assert('a' - 'A' == (1 << 5));
        return c | (IsUpperImpl(c) * ('a' - 'A'));
    }

    static constexpr bool IsUpperImpl(int32_t c) noexcept {
        return static_cast<uint32_t>(c) - 'A' <= 'Z' - 'A';
    }

    static constexpr bool IsInAlphabet(unsigned char symbol) noexcept {
        return static_cast<uint32_t>(symbol) - kAlphabetStart <=
               kAlphabetEnd - kAlphabetStart;
    }

    static constexpr bool IsInAlphabet(char symbol) noexcept {
        return IsInAlphabet(static_cast<unsigned char>(symbol));
    }

    static constexpr size_t SymbolToIndex(unsigned char symbol) noexcept {
        int32_t symbol_as_int = symbol;
        if constexpr (kIsCaseInsensetive) {
            // We don't use std::tolower because we know that all
            //  chars are < 128. On the other side, std::tolower makes
            //  text finding runs almost 1.5x times slower because of
            //  the locale handling.
            symbol_as_int = ToLowerImpl(symbol_as_int);
        }

        return static_cast<size_t>(static_cast<uint32_t>(symbol_as_int)) -
               kAlphabetStart;
    }

    static constexpr size_t SymbolToIndex(char symbol) noexcept {
        return SymbolToIndex(static_cast<unsigned char>(symbol));
    }

    template <class PatternIterator>
    static constexpr bool AddPatternImpl(
        PatternIterator pattern_iter_begin,
        PatternIterator pattern_iter_end, std::vector<Node>& nodes,
        std::vector<StoredPatternSize>& words_lengths) noexcept {
        const auto pattern_size =
            static_cast<size_t>(pattern_iter_end - pattern_iter_begin);
        size_t current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end;
             ++pattern_iter_begin) {
            size_t symbol_index = SymbolToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }
            size_t next_node_index =
                nodes[current_node_index][symbol_index];
            if (next_node_index != kNullNodeIndex) {
                current_node_index = next_node_index;
            } else {
                break;
            }
        }

        const auto lasted_max_length =
            static_cast<size_t>(pattern_iter_end - pattern_iter_begin);
        nodes.reserve(nodes.size() + lasted_max_length);
        for (size_t new_node_index = nodes.size();
             pattern_iter_begin != pattern_iter_end;
             ++pattern_iter_begin) {
            size_t symbol_index = SymbolToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }

            nodes.emplace_back();
            nodes[current_node_index][symbol_index] =
                static_cast<StoredNodeIndex>(new_node_index);
            current_node_index = new_node_index++;
        }

        StoredPatternIndex pattern_index =
            static_cast<StoredPatternIndex>(words_lengths.size());
        nodes[current_node_index].pattern_index = pattern_index;
        words_lengths.push_back(
            static_cast<StoredPatternSize>(pattern_size));
        return true;
    }

    template <class PatternIterator>
    static constexpr bool ContainsPatternImpl(
        PatternIterator pattern_iter_begin,
        PatternIterator pattern_iter_end,
        const std::vector<Node>& nodes) noexcept {
        size_t current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end;
             ++pattern_iter_begin) {
            size_t index = SymbolToIndex(*pattern_iter_begin);
            if (index >= kAlphabetLength) {
                return false;
            }

            size_t next_node_index = nodes[current_node_index][index];
            if (next_node_index != kNullNodeIndex) {
                current_node_index = next_node_index;
            } else {
                return false;
            }
        }

        return nodes[current_node_index].IsTerminal();
    }

    std::vector<Node> nodes_;
    std::vector<StoredPatternSize> patterns_lengths_;
};

}  // namespace actrie
