#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "../misc/config_macros.hpp"

namespace actrie {

using Symbol = unsigned char;

inline constexpr Symbol kDefaultAlphabetStart = 'A';
inline constexpr Symbol kDefaultAlphabetEnd = 'z';

template <Symbol AlphabetStart = kDefaultAlphabetStart,
          Symbol AlphabetEnd = kDefaultAlphabetEnd,
          bool IsCaseInsensetive = false>
class ACTrie;

template <Symbol AlphabetStart = kDefaultAlphabetStart,
          Symbol AlphabetEnd = kDefaultAlphabetEnd,
          bool IsCaseInsensetive = false>
class ReplacingACTrie;

template <Symbol AlphabetStart = kDefaultAlphabetStart,
          Symbol AlphabetEnd = kDefaultAlphabetEnd,
          bool IsCaseInsensetive = false>
class ACTrieBuilder;

template <Symbol AlphabetStart = kDefaultAlphabetStart,
          Symbol AlphabetEnd = kDefaultAlphabetEnd,
          bool IsCaseInsensetive = false>
class ReplacingACTrieBuilder;

template <Symbol AlphabetStart, Symbol AlphabetEnd, bool IsCaseInsensetive>
class [[nodiscard]] ACTrie {
public:
    friend class ACTrieBuilder<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;

    using size_type = std::size_t;
    using StoredNodeIndex = std::uint32_t;
    using StoredPatternSize = std::uint32_t;
    using StoredPatternIndex = std::uint32_t;

protected:
    static constexpr Symbol kAlphabetStart = AlphabetStart;
    static constexpr Symbol kAlphabetEnd = AlphabetEnd;
    static constexpr Symbol kIsCaseInsensetive = IsCaseInsensetive;

    static_assert('\0' < kAlphabetStart && kAlphabetStart < kAlphabetEnd &&
                      kAlphabetEnd < std::numeric_limits<char>::max(),
                  "Invalid alphabet boundaries");
    static constexpr Symbol kAlphabetLength = kAlphabetEnd - kAlphabetStart + 1;

    static constexpr StoredNodeIndex kNullNodeIndex = 0;
    static constexpr StoredNodeIndex kFakePrerootNodeIndex = kNullNodeIndex + 1;
    static constexpr StoredNodeIndex kRootNodeIndex = kFakePrerootNodeIndex + 1;

    using EdgesArray = std::array<StoredNodeIndex, kAlphabetLength>;

    [[nodiscard]] static constexpr EdgesArray CreateEdgesArray() noexcept {
        EdgesArray edges{};
        edges.fill(kNullNodeIndex);
        return edges;
    }

    struct Node final {
        static constexpr StoredPatternIndex kMissingWordIndex =
            std::numeric_limits<StoredPatternIndex>::max();

        EdgesArray edges = CreateEdgesArray();
        StoredNodeIndex suffix_link = kNullNodeIndex;
        StoredNodeIndex compressed_suffix_link = kNullNodeIndex;
        StoredPatternIndex pattern_index = kMissingWordIndex;

        [[nodiscard]] constexpr StoredNodeIndex operator[](const size_type index) const noexcept {
            return edges[index];
        }
        [[nodiscard]] constexpr StoredNodeIndex& operator[](const size_type index) noexcept {
            return edges[index];
        }
        [[nodiscard]] constexpr bool IsTerminal() const noexcept {
            return pattern_index != kMissingWordIndex;
        }
    };

public:
    [[nodiscard]] constexpr bool ContainsPattern(const std::string_view pattern) const noexcept {
        return ContainsPatternImpl(pattern.begin(), pattern.end(), nodes_);
    }
    template <typename FindCallback>
        requires requires(FindCallback func,
                          std::string_view found_word,
                          size_type start_index_in_original_text) {
            func(found_word, start_index_in_original_text);
        }
    constexpr void FindAllSubstringsInText(const std::string_view text,
                                           FindCallback find_callback) const {
        if constexpr (std::is_convertible_v<decltype(find_callback), bool>) {
            assert(find_callback);
        }

        StoredNodeIndex current_node_index = kRootNodeIndex;
        for (size_type i = 0; i < text.size(); i++) {
            const size_type symbol_index = CharToIndex(text[i]);
            if (symbol_index >= kAlphabetLength) {
                current_node_index = kRootNodeIndex;
                continue;
            }

            current_node_index = nodes_[current_node_index][symbol_index];
            assert(current_node_index != kNullNodeIndex);

            const Node& node = nodes_[current_node_index];
            if (node.IsTerminal()) {
                const size_type pattern_index = node.pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }

                const size_type pattern_size = patterns_lengths_[pattern_index];
                const size_type occurance_start_index = i + 1 - pattern_size;

                find_callback(text.substr(occurance_start_index, pattern_size),
                              occurance_start_index);
            }

            for (StoredNodeIndex terminal_node_index = node.compressed_suffix_link;
                 terminal_node_index != kRootNodeIndex;
                 terminal_node_index = nodes_[terminal_node_index].compressed_suffix_link) {
                assert(terminal_node_index != kNullNodeIndex);
                assert(nodes_[terminal_node_index].IsTerminal());

                const size_type pattern_index = nodes_[terminal_node_index].pattern_index;
                assert(pattern_index < patterns_lengths_.size());

                const size_type pattern_size = patterns_lengths_[pattern_index];
                const size_type occurance_start_index = i + 1 - pattern_size;

                find_callback(text.substr(occurance_start_index, pattern_size),
                              occurance_start_index);
            }
        }
    }

    struct FoundOccurance {
        std::string_view found_word;
        size_t start_index_in_original_text;
    };

    template <std::ranges::range Container = std::vector<FoundOccurance>>
    [[nodiscard]] constexpr Container CollectAllSubstringsFromText(
        const std::string_view text ATTRIBUTE_LIFETIME_BOUND) const {
        Container c;
        FindAllSubstringsInText(text, [&c](const std::string_view found_word,
                                           const size_type start_index_in_original_text) {
            c.emplace_back(found_word, start_index_in_original_text);
        });
        return c;
    }

    template <bool IsExactWordsMatching = true,
              bool CountEmptyLines = true,
              Symbol LinesDelimeter = '\n',
              typename QueryWordCallback,
              typename NewLineCallback>
        requires requires(QueryWordCallback func,
                          const size_type line_number,
                          const StoredPatternIndex query_word_index) {
            func(line_number, query_word_index);
        } && requires(NewLineCallback func,
                      const size_type line_number,
                      const size_type words_on_current_line,
                      const size_type line_start_index,
                      const size_type line_end_index) {
            func(line_number, words_on_current_line, line_start_index, line_end_index);
        }
    [[nodiscard]]
    constexpr size_type FindAllSubstringsInTextAndCountLines(std::string_view text,
                                                             QueryWordCallback find_callback,
                                                             NewLineCallback line_callback) const {
        static_assert(!(kAlphabetStart <= LinesDelimeter && LinesDelimeter <= kAlphabetEnd) &&
                          SymbolToIndex(LinesDelimeter) >= kAlphabetLength,
                      "Lines delimeter can\'t be in the alphabet");

        if constexpr (std::is_convertible_v<decltype(find_callback), bool>) {
            assert(find_callback);
        }
        if constexpr (std::is_convertible_v<decltype(line_callback), bool>) {
            assert(line_callback);
        }

        StoredNodeIndex current_node_index = kRootNodeIndex;
        size_type line_start_index = 0;
        size_type current_line = 1;
        size_type words_on_current_line = 0;
        size_type lines_count = 0;
        bool prev_symbol_in_alphabet = false;
        for (size_type i = 0; i < text.size(); i++) {
            const Symbol symbol = CharToSymbol(text[i]);
            const size_type symbol_index = SymbolToIndex(symbol);
            if (symbol_index >= kAlphabetLength) {
                current_node_index = kRootNodeIndex;
                words_on_current_line += size_type{prev_symbol_in_alphabet};
                if (symbol == LinesDelimeter) {
                    assert(line_start_index <= i);
                    if (CountEmptyLines || line_start_index != i) {
                        line_callback(current_line, words_on_current_line, line_start_index, i);
                        lines_count++;
                    }
                    current_line++;
                    line_start_index = i + 1;
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
                const StoredPatternIndex pattern_index = node.pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }

                if constexpr (IsExactWordsMatching) {
                    const size_type pattern_size = patterns_lengths_[pattern_index];
                    const size_type word_begin_index = i + 1 - pattern_size;
                    const bool prev_symbol_and_next_symbol_not_in_the_alphabet =
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
            for (StoredNodeIndex terminal_node_index = node.compressed_suffix_link;
                 terminal_node_index != kRootNodeIndex;
                 terminal_node_index = nodes_[terminal_node_index].compressed_suffix_link) {
                if (!std::is_constant_evaluated()) {
                    assert(terminal_node_index != kNullNodeIndex &&
                           nodes_[terminal_node_index].IsTerminal());
                }
                const StoredPatternIndex pattern_index = nodes_[terminal_node_index].pattern_index;
                if (!std::is_constant_evaluated()) {
                    assert(pattern_index < patterns_lengths_.size());
                }

                if constexpr (IsExactWordsMatching) {
                    const size_type pattern_size = patterns_lengths_[pattern_index];
                    const size_type l = i + 1 - pattern_size;
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
    [[nodiscard]] constexpr size_type PatternsSize() const noexcept {
        return patterns_lengths_.size();
    }

protected:
    constexpr ACTrie(std::vector<Node>&& nodes,
                     std::vector<StoredPatternSize>&& words_lengths) noexcept
        : nodes_(std::move(nodes)), patterns_lengths_(std::move(words_lengths)) {}

    template <std::input_iterator PatternIterator>
    [[nodiscard]]
    static constexpr bool ContainsPatternImpl(PatternIterator pattern_iter_begin,
                                              const PatternIterator pattern_iter_end,
                                              const std::vector<Node>& nodes) noexcept {
        size_type current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end; ++pattern_iter_begin) {
            const size_type index = CharToIndex(*pattern_iter_begin);
            if (index >= kAlphabetLength) {
                return false;
            }

            assert(current_node_index < nodes.size());
            const size_type next_node_index = nodes[current_node_index][index];
            if (next_node_index == kNullNodeIndex) {
                return false;
            }

            current_node_index = next_node_index;
        }

        assert(current_node_index < nodes.size());
        return nodes[current_node_index].IsTerminal();
    }

    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr bool IsInAlphabet(const Symbol symbol) noexcept {
        return SymbolToUInt(symbol) - kAlphabetStart <= kAlphabetEnd - kAlphabetStart;
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr bool IsInAlphabet(const char symbol) noexcept {
        return IsInAlphabet(CharToSymbol(symbol));
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr size_type SymbolToIndex(const Symbol symbol) noexcept {
        std::uint32_t symbol_as_int = SymbolToUInt(symbol);
        if constexpr (kIsCaseInsensetive) {
            // We don't use std::tolower because we know that all
            //  chars are < 128. What's more important, std::tolower makes
            //  text finding run almost 1.5x times slower because of
            //  the locale handling.
            symbol_as_int = ToLowerImpl(symbol_as_int);
        }

        return size_type{symbol_as_int} - kAlphabetStart;
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr std::uint32_t SymbolToUInt(const Symbol symbol) noexcept {
        return std::uint32_t{symbol};
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr size_type CharToIndex(const char chr) noexcept {
        return SymbolToIndex(CharToSymbol(chr));
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr Symbol CharToSymbol(const char chr) noexcept {
        static_assert(std::is_same_v<Symbol, unsigned char>);
        return static_cast<Symbol>(chr);
    }

private:
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr std::uint32_t ToLowerImpl(const std::uint32_t c) noexcept {
        return c | (IsUpperImpl(c) * std::uint32_t{'a' - 'A'});
    }
    [[nodiscard]]
    ATTRIBUTE_CONST static constexpr bool IsUpperImpl(const std::uint32_t c) noexcept {
        return c - 'A' <= 'Z' - 'A';
    }

protected:
    std::vector<Node> nodes_;
    std::vector<StoredPatternSize> patterns_lengths_;
};

template <Symbol AlphabetStart, Symbol AlphabetEnd, bool IsCaseInsensetive>
class [[nodiscard]] ReplacingACTrie final
    : private ACTrie<AlphabetStart, AlphabetEnd, IsCaseInsensetive> {
private:
    using Base = ACTrie<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;
    using Node = typename Base::Node;
    friend class ReplacingACTrieBuilder<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;

public:
    using size_type = typename Base::size_type;
    using StoredNodeIndex = typename Base::StoredNodeIndex;
    using StoredPatternSize = typename Base::StoredPatternSize;
    using StoredPatternIndex = typename Base::StoredPatternIndex;

    using Base::ContainsPattern;
    using Base::FindAllSubstringsInText;
    using Base::FindAllSubstringsInTextAndCountLines;
    using Base::PatternsSize;

    static constexpr size_type kAllOccurances = std::numeric_limits<size_type>::max();

    size_type ReplaceAllOccurances(std::string& text) const {
        return ReplaceAtMostKOccurances(text, kAllOccurances);
    }

    size_type ReplaceAtMostKOccurances(std::string& text, const size_type max_replacements) const {
        size_type replaced_occurances = 0;
        switch (max_replacements) {
            case 1:
                replaced_occurances += ReplaceFirstOccurance(text) ? size_type{1} : size_type{0};
                [[fallthrough]];
            case 0:
                return replaced_occurances;
            default:
                break;
        }

        struct ReplacementInfo {
            size_type l_index_in_text{};
            size_type pattern_index{};
        };

        using ReplacementInfoVector = std::vector<ReplacementInfo>;

        size_type new_length = text.size();
        ReplacementInfoVector planned_replacements;
        StoredNodeIndex current_node_index = Base::kRootNodeIndex;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter) {
            const size_type symbol_index = Base::CharToIndex(*iter);
            if (symbol_index >= Base::kAlphabetLength) {
                current_node_index = Base::kRootNodeIndex;
                continue;
            }

            current_node_index = this->nodes_[current_node_index][symbol_index];
            assert(current_node_index != Base::kNullNodeIndex);
            const Node& current_node = Base::nodes_[current_node_index];
            const bool current_node_is_terminal = current_node.IsTerminal();
            const StoredNodeIndex compressed_suffix_link = current_node.compressed_suffix_link;

            const bool no_match =
                !current_node_is_terminal && compressed_suffix_link == Base::kRootNodeIndex;
            if (no_match) {
                continue;
            }

            assert(current_node_is_terminal || this->nodes_[compressed_suffix_link].IsTerminal());
            const StoredPatternIndex pattern_index =
                current_node_is_terminal ? current_node.pattern_index
                                         : this->nodes_[compressed_suffix_link].pattern_index;

            const size_type r_index_including = static_cast<size_type>(iter - text.begin());
            assert(pattern_index < this->patterns_lengths_.size());
            const StoredPatternSize pattern_length = this->patterns_lengths_[pattern_index];
            const size_type l_index_including = r_index_including + 1 - pattern_length;
            const std::string& replacement = words_replacements_[pattern_index];

            const bool replace_inplace =
                planned_replacements.empty() && pattern_length == replacement.size();
            if (replace_inplace) {
                std::char_traits<char>::copy(text.data() + l_index_including, replacement.data(),
                                             pattern_length);
                replaced_occurances++;
            } else {
                planned_replacements.push_back(ReplacementInfo{
                    .l_index_in_text = l_index_including,
                    .pattern_index = pattern_index,
                });
                new_length += (replacement.size() - pattern_length);
            }

            assert(replaced_occurances + planned_replacements.size() <= max_replacements);
            if (replaced_occurances + planned_replacements.size() == max_replacements) {
                break;
            }

            current_node_index = Base::kRootNodeIndex;
        }

        if (text.size() == new_length) {
            assert(planned_replacements.empty());
            return replaced_occurances;
        }

        size_type right_boundary = text.size();
        size_type right_offset = 0;
        if (new_length > text.size()) {
            text.resize(new_length);
        }

        assert(replaced_occurances + planned_replacements.size() <= max_replacements);
        const auto reverse =
            [](const ReplacementInfoVector& vec ATTRIBUTE_LIFETIME_BOUND) noexcept {
                using Iterator = typename ReplacementInfoVector::const_reverse_iterator;
                struct RevStruct final {
                    Iterator begin_iter;
                    Iterator end_iter;

                    [[nodiscard]] Iterator begin() const noexcept {
                        return begin_iter;
                    }
                    [[nodiscard]] Iterator end() const noexcept {
                        return end_iter;
                    }
                };

                return RevStruct{
                    .begin_iter = vec.rbegin(),
                    .end_iter = vec.rend(),
                };
            };
        // std::ranges::reverse fails to call `begin(planned_replacements)` on clang 14.0.0
        for (const ReplacementInfo& info : reverse(planned_replacements)) {
            const size_type pattern_index = info.pattern_index;
            const StoredPatternSize pattern_size = Base::patterns_lengths_[pattern_index];
            const size_type l_index_in_text = info.l_index_in_text;
            const size_type r_index_in_text = l_index_in_text + pattern_size;
            size_type moved_part_length = right_boundary - r_index_in_text;
            char* dst_address = text.data() + (new_length - right_offset - moved_part_length);
            const char* const src_address = text.data() + r_index_in_text;
            if (dst_address != src_address) {
                std::char_traits<char>::move(dst_address, src_address, moved_part_length);
            }

            size_type replacement_length = words_replacements_[pattern_index].size();
            dst_address -= replacement_length;

            std::char_traits<char>::copy(dst_address, words_replacements_[pattern_index].c_str(),
                                         replacement_length);
            replaced_occurances++;

            right_boundary = l_index_in_text;
            right_offset += moved_part_length + replacement_length;
        }

        if (new_length < text.size()) {
            text.resize(new_length);
        }

        return replaced_occurances;
    }

    constexpr bool ReplaceFirstOccurance(std::string& text) const {
        StoredNodeIndex current_node_index = Base::kRootNodeIndex;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter) {
            const size_type symbol_index = Base::CharToIndex(*iter);
            if (symbol_index >= Base::kAlphabetLength) {
                current_node_index = Base::kRootNodeIndex;
                continue;
            }

            current_node_index = this->nodes_[current_node_index][symbol_index];
            assert(current_node_index != Base::kNullNodeIndex);
            const Node& current_node = this->nodes_[current_node_index];
            bool current_node_is_terminal = current_node.IsTerminal();
            StoredPatternIndex compressed_suffix_link = current_node.compressed_suffix_link;

            const bool no_match =
                !current_node_is_terminal && compressed_suffix_link == Base::kRootNodeIndex;
            if (no_match) {
                continue;
            }
            assert(current_node_is_terminal || this->nodes_[compressed_suffix_link].IsTerminal());
            StoredPatternIndex pattern_index =
                current_node_is_terminal ? current_node.pattern_index
                                         : this->nodes_[compressed_suffix_link].pattern_index;
            ReplaceOccuranceWithResize(text, iter, pattern_index, this->patterns_lengths_,
                                       words_replacements_);
            return true;
        }

        return false;
    }

private:
    using ReplacementsVector = std::vector<std::string>;

    static constexpr void ReplaceOccuranceWithResize(
        std::string& text,
        std::string::iterator occurance_last_position_iter,
        StoredPatternIndex pattern_index,
        const std::vector<StoredPatternSize>& patterns_lengths,
        const ReplacementsVector& words_replacements) {
        const size_type r_index_including =
            static_cast<size_type>(occurance_last_position_iter - text.begin());
        assert(pattern_index < patterns_lengths.size());
        const StoredPatternSize pattern_size = patterns_lengths[pattern_index];
        const size_type l_index_including = r_index_including + 1 - pattern_size;
        const std::string& replacement = words_replacements[pattern_index];

        if (replacement.size() != pattern_size) {
            text.resize(text.size() - pattern_size + replacement.size());
            std::char_traits<char>::move(text.data() + l_index_including + replacement.size(),
                                         std::as_const(text).data() + r_index_including + 1,
                                         text.size() - 1 - r_index_including);
        }

        std::char_traits<char>::copy(text.data() + l_index_including, replacement.data(),
                                     replacement.size());
    }
    constexpr ReplacingACTrie(std::vector<Node>&& nodes,
                              std::vector<StoredPatternSize>&& words_lengths,
                              ReplacementsVector&& words_replacements) noexcept
        : Base(std::move(nodes), std::move(words_lengths))
        , words_replacements_(std::move(words_replacements)) {}

    ReplacementsVector words_replacements_;
};

// cppcheck-suppress-begin [duplInheritedMember]

template <Symbol AlphabetStart, Symbol AlphabetEnd, bool IsCaseInsensetive>
class [[nodiscard]] ACTrieBuilder {
public:
    using ACTrieType = ACTrie<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;
    using size_type = typename ACTrieType::size_type;
    using StoredNodeIndex = typename ACTrieType::StoredNodeIndex;
    using StoredPatternSize = typename ACTrieType::StoredPatternSize;
    using StoredPatternIndex = typename ACTrieType::StoredPatternIndex;

protected:
    using Node = typename ACTrieType::Node;

    static constexpr auto kAlphabetStart = ACTrieType::kAlphabetStart;
    static constexpr auto kAlphabetEnd = ACTrieType::kAlphabetEnd;
    static constexpr auto kAlphabetLength = ACTrieType::kAlphabetLength;
    static constexpr auto kIsCaseInsensetive = ACTrieType::kIsCaseInsensetive;
    static constexpr auto kNullNodeIndex = ACTrieType::kNullNodeIndex;
    static constexpr auto kFakePrerootNodeIndex = ACTrieType::kFakePrerootNodeIndex;
    static constexpr auto kRootNodeIndex = ACTrieType::kRootNodeIndex;

public:
    constexpr ACTrieBuilder() : nodes_(), patterns_lengths_() {
        constexpr size_type kDefaultNodesSize =
            std::max({kNullNodeIndex, kFakePrerootNodeIndex, kRootNodeIndex}) + 1;
        constexpr size_type kDefaultNodesCapacity = std::max(kDefaultNodesSize, size_type{32});

        nodes_.reserve(kDefaultNodesCapacity);
        nodes_.resize(kDefaultNodesSize);
    }
    static constexpr ACTrieBuilder WithCapacity(const size_type patterns_capacity) {
        ACTrieBuilder builder;
        builder.patterns_lengths_.reserve(patterns_capacity);
        return builder;
    }

    [[nodiscard]] constexpr size_type PatternsSize() const noexcept {
        return patterns_lengths_.size();
    }
    constexpr bool AddPattern(const std::string_view pattern) {
        return this->AddPatternImpl(pattern.begin(), pattern.end(), nodes_, patterns_lengths_);
    }
    [[nodiscard]] constexpr bool ContainsPattern(std::string_view pattern) const noexcept {
        return ACTrieType::ContainsPatternImpl(pattern.begin(), pattern.end(), nodes_);
    }
    [[nodiscard]] constexpr ACTrieType Build() && {
        this->ComputeLinksForNodes(nodes_);
        return ACTrieType(std::move(nodes_), std::move(patterns_lengths_));
    }

protected:
    static constexpr void ComputeLinksForNodes(std::vector<Node>& nodes) {
        nodes.at(kRootNodeIndex).suffix_link = kFakePrerootNodeIndex;
        nodes[kRootNodeIndex].compressed_suffix_link = kRootNodeIndex;
        nodes[kFakePrerootNodeIndex].edges.fill(kRootNodeIndex);

        constexpr size_type kInvalidIndex = std::numeric_limits<size_type>::max();

        // std::vector is used instead of std::queue
        //  in order to make the method constexpr.
        std::vector<size_type> bfs_queue(nodes.size(), kInvalidIndex);
        size_type queue_head = 0;
        size_type queue_tail = 0;
        bfs_queue[queue_tail++] = kRootNodeIndex;
        do {
            const size_type node_index = bfs_queue[queue_head++];
            assert(node_index != kInvalidIndex);
            ComputeLinksForNodeChildren(node_index, nodes, bfs_queue, queue_tail);
        } while (queue_head < queue_tail);
    }

private:
    static constexpr void ComputeLinksForNodeChildren(const size_type node_index,
                                                      const std::span<Node> nodes,
                                                      const std::span<size_type> bfs_queue,
                                                      size_type& queue_tail) noexcept {
        assert(node_index < nodes.size());
        Node& node = nodes[node_index];
        for (size_type symbol_index = 0; symbol_index < kAlphabetLength; symbol_index++) {
            assert(node.suffix_link < nodes.size());
            const StoredNodeIndex child_link_v_index = nodes[node.suffix_link][symbol_index];
            const size_type child_index = node[symbol_index];
            if (child_index == kNullNodeIndex) {
                node[symbol_index] = child_link_v_index;
                continue;
            }

            assert(child_index < nodes.size());
            nodes[child_index].suffix_link = child_link_v_index;
            assert(child_link_v_index < nodes.size());
            const StoredNodeIndex child_comp_sl = nodes[child_link_v_index].compressed_suffix_link;
            const bool is_terminal_or_root =
                nodes[child_link_v_index].IsTerminal() || child_link_v_index == kRootNodeIndex;
            nodes[child_index].compressed_suffix_link =
                is_terminal_or_root ? child_link_v_index : child_comp_sl;
            assert(queue_tail < bfs_queue.size());
            bfs_queue[queue_tail++] = child_index;
        }
    }

    template <std::random_access_iterator PatternIterator>
    static constexpr bool AddPatternImpl(PatternIterator pattern_iter_begin,
                                         const PatternIterator pattern_iter_end,
                                         std::vector<Node>& nodes,
                                         std::vector<StoredPatternSize>& words_lengths) {
        const auto pattern_size = static_cast<size_type>(pattern_iter_end - pattern_iter_begin);
        size_type current_node_index = kRootNodeIndex;
        for (; pattern_iter_begin != pattern_iter_end; ++pattern_iter_begin) {
            const size_type symbol_index = ACTrieType::CharToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }

            assert(current_node_index < nodes.size());
            const size_type next_node_index = nodes[current_node_index][symbol_index];
            if (next_node_index == kNullNodeIndex) {
                break;
            }

            current_node_index = next_node_index;
        }

        const auto lasted_max_length =
            static_cast<size_type>(pattern_iter_end - pattern_iter_begin);
        nodes.reserve(nodes.size() + lasted_max_length);
        for (size_type new_node_index = nodes.size(); pattern_iter_begin != pattern_iter_end;
             ++pattern_iter_begin) {
            const size_type symbol_index = ACTrieType::CharToIndex(*pattern_iter_begin);
            if (symbol_index >= kAlphabetLength) {
                return false;
            }

            nodes.emplace_back();
            assert(current_node_index < nodes.size());
            nodes[current_node_index][symbol_index] = static_cast<StoredNodeIndex>(new_node_index);
            current_node_index = new_node_index++;
        }

        const StoredPatternIndex pattern_index =
            static_cast<StoredPatternIndex>(words_lengths.size());
        assert(current_node_index < nodes.size());
        nodes[current_node_index].pattern_index = pattern_index;
        words_lengths.push_back(static_cast<StoredPatternSize>(pattern_size));
        return true;
    }

protected:
    std::vector<Node> nodes_;
    std::vector<StoredPatternSize> patterns_lengths_;
};

template <Symbol AlphabetStart, Symbol AlphabetEnd, bool IsCaseInsensetive>
class [[nodiscard]] ReplacingACTrieBuilder final
    : private ACTrieBuilder<AlphabetStart, AlphabetEnd, IsCaseInsensetive> {
private:
    using Base = ACTrieBuilder<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;
    using Node = typename Base::Node;

public:
    using ACTrieType = ReplacingACTrie<AlphabetStart, AlphabetEnd, IsCaseInsensetive>;
    using size_type = typename Base::size_type;
    using StoredNodeIndex = typename Base::StoredNodeIndex;
    using StoredPatternSize = typename Base::StoredPatternSize;
    using StoredPatternIndex = typename Base::StoredPatternIndex;

    constexpr ReplacingACTrieBuilder() : words_replacements_() {}

    static constexpr ReplacingACTrieBuilder WithCapacity(const size_type patterns_capacity) {
        ReplacingACTrieBuilder builder;
        builder.patterns_lengths_.reserve(patterns_capacity);
        builder.words_replacements_.reserve(patterns_capacity);
        return builder;
    }

    using Base::ContainsPattern;
    using Base::PatternsSize;

    bool AddPatternWithReplacement(const std::string_view pattern, std::string replacement) {
        const bool added = Base::AddPattern(pattern);
        if (likely(added)) {
#if CONFIG_GNUC_AT_LEAST(14, 0) && !defined(__clang__)
// Bug in GCC 14+: false positive may occur with
//  warning -Walloc-size-larger-than=x if x < 9223372036854775776
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wno-alloc-size-larger-than"
#endif
            words_replacements_.push_back(std::move(replacement));
#if CONFIG_GNUC_AT_LEAST(14, 0) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
        }

        return added;
    }
    [[nodiscard]] constexpr ACTrieType Build() && {
        Base::ComputeLinksForNodes(Base::nodes_);
        return ACTrieType(std::move(Base::nodes_), std::move(Base::patterns_lengths_),
                          std::move(words_replacements_));
    }

private:
    std::vector<std::string> words_replacements_;
};

// cppcheck-suppress-end [duplInheritedMember]

}  // namespace actrie
