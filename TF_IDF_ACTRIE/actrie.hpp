#ifndef _ACTRIE_H_
#define _ACTRIE_H_ 1

#include <cassert>     // assert
#include <climits>     // CHAR_MAX
#include <cstdint>     // std::uint32_t, std::size_t
#include <cstring>     // std::memset
#include <deque>       // std::deque
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace ACTrieADS {

template <uint8_t ALPHABET_START = 'a', uint8_t ALPHABET_END = 'z', bool IsCaseInsensetive = false>
class ACTrie final {
    static_assert('\0' < ALPHABET_START && ALPHABET_START < ALPHABET_END && ALPHABET_END < CHAR_MAX);

    // Default value = 'z' - 'a' + 1 = 26
    static constexpr uint8_t ALPHABET_LENGTH = ALPHABET_END - ALPHABET_START + 1;

    static constexpr size_t NULL_NODE_INDEX = 0;
    static constexpr size_t FAKE_PREROOT_INDEX = 1;
    static constexpr size_t ROOT_INDEX = 2;
    static constexpr size_t DEFAULT_NODES_COUNT = 3; // null node; fake preroot node; root node
    static constexpr size_t DEFAULT_VECTORS_CAPACITY = DEFAULT_NODES_COUNT * 2;

    static_assert(std::max(NULL_NODE_INDEX, std::max(FAKE_PREROOT_INDEX, ROOT_INDEX)) < DEFAULT_NODES_COUNT);

    struct ACTNode {
        static constexpr uint32_t MISSING_SENTIEL = static_cast<uint32_t>(-1);

        // Indexes in array of nodes
        uint32_t edges[ALPHABET_LENGTH];

        // Index in array of nodes
        uint32_t suffix_link = NULL_NODE_INDEX;

        // Index in array of nodes
        uint32_t compressed_suffix_link = NULL_NODE_INDEX;

        /* 
        * Index of the word in the ac trie which ends on this
        * MISSING_SENTIEL if node is not terminal
        */
        uint32_t word_index = MISSING_SENTIEL;

        constexpr ACTNode() noexcept {
            std::memset(edges, static_cast<int>(NULL_NODE_INDEX), sizeof(edges));
        }

        constexpr bool IsTerminal() const noexcept {
            return word_index != MISSING_SENTIEL;
        }
    };

    std::vector<ACTNode> nodes_;
    std::vector<uint32_t> words_lengths_;
    mutable bool are_links_computed_ = false;

public:
    constexpr ACTrie() : nodes_{}, words_lengths_() {
        nodes_.reserve(DEFAULT_VECTORS_CAPACITY);
        nodes_.resize(DEFAULT_NODES_COUNT);
        words_lengths_.reserve(DEFAULT_VECTORS_CAPACITY);

        /*
         * link(root) = fake_vertex;
         * For all chars from the alphabet: fake_vertex ---char--> root
         */

        nodes_[ROOT_INDEX].suffix_link = FAKE_PREROOT_INDEX;
        nodes_[ROOT_INDEX].compressed_suffix_link = ROOT_INDEX;

        for (uint32_t& edge : nodes_[FAKE_PREROOT_INDEX].edges) {
            edge = ROOT_INDEX;
        }
    }

    constexpr void ReservePlaceForPatterns(size_t patterns_capacity) {
        words_lengths_.reserve(patterns_capacity);
    }

    void AddPattern(std::string_view pattern) {
        assert(!IsReady());

        uint32_t current_node_index = ROOT_INDEX;
        const char* pattern_iter = pattern.begin();
        const char* const pattern_end = pattern.end();

        for (; pattern_iter != pattern_end; ++pattern_iter) {
            char sigma;
            if constexpr (IsCaseInsensetive) {
                sigma = ToLower(*pattern_iter);
            }
            else {
                sigma = *pattern_iter;
            }

            if (!IsInAlphabet(sigma)) {
                assert(!"char in pattern is not in alphabet!!!");
                continue;
            }

            uint32_t next_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            if (next_node_index != NULL_NODE_INDEX) {
                current_node_index = next_node_index;
            }
            else {
                break;
            }
        }

        size_t lasted_max_length = static_cast<size_t>(pattern_end - pattern_iter);
        nodes_.reserve(nodes_.size() + lasted_max_length);

        /*
         * Inserts substring [i..length - 1] of pattern if i < length (<=> i != length)
         * If i == length, then for cycle will no execute
         */
        for (; pattern_iter != pattern_end; ++pattern_iter) {
            char sigma;
            if constexpr (IsCaseInsensetive) {
                sigma = ToLower(*pattern_iter);
            }
            else {
                sigma = *pattern_iter;
            }

            if (!IsInAlphabet(sigma)) {
                assert(!"char in pattern is not in alphabet!!!");
                continue;
            }

            uint32_t new_node_index = static_cast<uint32_t>(nodes_.size());
            nodes_.emplace_back();
            nodes_[current_node_index].edges[CharToEdgeIndex(sigma)] = new_node_index;
            current_node_index = new_node_index;
        }

        uint32_t word_index = static_cast<uint32_t>(words_lengths_.size());
        nodes_[current_node_index].word_index = word_index;
        words_lengths_.push_back(static_cast<uint32_t>(pattern.size()));
    }

    /*
     * Aho–Corasick deterministic finite-state machine is built on the ordinal trie.
     */
    constexpr bool ContainsPattern(std::string_view pattern) const noexcept {
        uint32_t current_node_index = ROOT_INDEX;
        for (char sigma : pattern) {
            if constexpr (IsCaseInsensetive) {
                sigma = ToLower(sigma);
            }

            if (!IsInAlphabet(sigma)) {
                return false;
            }

            uint32_t next_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            if (next_node_index != NULL_NODE_INDEX) {
                current_node_index = next_node_index;
            }
            else {
                return false;
            }
        }

        return nodes_[current_node_index].IsTerminal();
    }

    void ComputeLinks() {
        assert(!IsReady());

        /*
         * See MIPT lecture https://youtu.be/MEFrIcGsw1o for more info
         *
         * For each char (marked as sigma) in the Alphabet:
         *   v := root_eges[sigma] <=> to((root, sigma))
         *
         *   root_edges[c] = root_edges[c] ? root_edegs[c] : root
         *   <=>
         *   to((root, sigma)) = to((root, sigma)) if (root, sigma) in rng(to) else root
         *
         *   link(v) = root (if v aka to((root, sigma)) exists)
         *
         *   rood_edges[sigma].compressed_suffix_link = root
		 */

        // Run BFS through all nodes.
        std::deque<uint32_t> bfs_queue;
        bfs_queue.push_back(ROOT_INDEX);

        do {
            uint32_t vertex_index = bfs_queue.front();
            bfs_queue.pop_front();

            // to(v, sigma) === vertex.edges[sigma]
            uint32_t* vertex_edges = nodes_[vertex_index].edges;
            uint32_t vertex_suffix_link = nodes_[vertex_index].suffix_link;

            assert(vertex_suffix_link != NULL_NODE_INDEX);
            // to((link(v), sigma)) === nodes[vertex.suffix_link].edges[sigma]
            const uint32_t* vertex_suffix_link_edges = nodes_[vertex_suffix_link].edges;

            // For each char (sigma) in the Alphabet vertex_edges[sigma] is the child such: v --sigma--> child
            for (size_t sigma = 0; sigma != ALPHABET_LENGTH; ++sigma) {
                uint32_t child_link_v_index = vertex_suffix_link_edges[sigma];
                assert(child_link_v_index != NULL_NODE_INDEX);

                // child = to(v, sigma)
                uint32_t child_index = vertex_edges[sigma];

                // to((v, sigma)) = to((v, sigma)) if (v, sigma) in the rng(to) else to((link(v), sigma))
                // rng(to) is a range of function 'to'
                if (child_index != NULL_NODE_INDEX) {
                    // link(to(v, sigma)) = to((link(v), sigma)) if (v, sigma) in the rng(to)
                    nodes_[child_index].suffix_link = child_link_v_index;

                    assert(nodes_[child_link_v_index].compressed_suffix_link != NULL_NODE_INDEX);

                    // comp(v) = link(v) if link(v) is terminal or root else comp(link(v))
                    nodes_[child_index].compressed_suffix_link =
                        ((!nodes_[child_link_v_index].IsTerminal()) & (child_link_v_index != ROOT_INDEX))
                        ? nodes_[child_link_v_index].compressed_suffix_link
                        : child_link_v_index;

                    bfs_queue.push_back(child_index);
                } else {
                    vertex_edges[sigma] = child_link_v_index;
                }
            }
        } while (!bfs_queue.empty());

#ifndef NDEBUG
        CheckComputedLinks();
#else
        are_links_computed_ = true;
#endif
    }

    constexpr bool IsReady() const noexcept {
        return are_links_computed_;
    }

    constexpr size_t NodesSize() const noexcept {
        return nodes_.size();
    }

    constexpr size_t PatternsSize() const noexcept {
        return words_lengths_.size();
    }

    template <typename FindCallback>
    requires requires (FindCallback func, std::string_view found_word, size_t start_index_in_original_text) {
        func(found_word, start_index_in_original_text);
    }
    void RunText(std::string_view text, FindCallback find_callback) const {
        assert(IsReady());

        if constexpr (std::is_pointer_v<decltype(find_callback)>) {
            assert(find_callback != nullptr);
        }

        uint32_t current_node_index = ROOT_INDEX;
        size_t i = 0;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter, ++i) {
            char sigma;
            if constexpr (IsCaseInsensetive) {
                sigma = ToLower(*iter);
            }
            else {
                sigma = *iter;
            }

            if (!IsInAlphabet(sigma)) {
                current_node_index = ROOT_INDEX;
                continue;
            }

            current_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            assert(current_node_index != NULL_NODE_INDEX);
            if (nodes_[current_node_index].IsTerminal()) {
                uint32_t word_index = nodes_[current_node_index].word_index;
                assert(word_index < words_lengths_.size());
                size_t word_length = words_lengths_[word_index];
                size_t l = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }

            // Jump up through compressed suffix links
            for (uint32_t tmp_node_index = nodes_[current_node_index].compressed_suffix_link;
                tmp_node_index != ROOT_INDEX;
                tmp_node_index = nodes_[tmp_node_index].compressed_suffix_link) {
                assert(tmp_node_index != NULL_NODE_INDEX && nodes_[tmp_node_index].IsTerminal());
                size_t word_index = nodes_[tmp_node_index].word_index;
                assert(word_index < words_lengths_.size());
                size_t word_length = words_lengths_[word_index];
                size_t l = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }
        }
    }

    template <bool IsExactWordsMatching = true, char LinesDelimeter = '\n', typename QueryWordCallback, typename NewLineCallback>
    requires requires (QueryWordCallback func, size_t line_number, size_t query_word_index) {
        func(line_number, query_word_index);
    }
    && requires (NewLineCallback func, size_t line_number, size_t words_on_current_line, size_t line_start_index, size_t line_end_index) {
        func(line_number, words_on_current_line, line_start_index, line_end_index);
    }
    size_t RunTextCountLines(std::string_view text, QueryWordCallback find_callback, NewLineCallback line_callback) const {
        static_assert(!IsInAlphabet(LinesDelimeter));
        assert(IsReady());

        if constexpr (std::is_pointer_v<decltype(find_callback)>) {
            assert(find_callback != nullptr);
        }

        uint32_t current_node_index = ROOT_INDEX;
        size_t line_start_index = 0;
        size_t i = 0;
        size_t current_line = 1;
        size_t words_on_current_line = 0;
        bool prev_symbol_in_alphabet = false;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter, ++i) {
            char sigma;
            if constexpr (IsCaseInsensetive) {
                sigma = ToLower(*iter);
            }
            else {
                sigma = *iter;
            }

            if (!IsInAlphabet(sigma)) {
                current_node_index = ROOT_INDEX;
                words_on_current_line += prev_symbol_in_alphabet;
                if (sigma == LinesDelimeter) {
                    if (line_start_index != i) {
                        line_callback(current_line, words_on_current_line, line_start_index, i);

                    }
                    line_start_index = i + 1;
                    words_on_current_line = 0;
                    current_line++;
                }

                prev_symbol_in_alphabet = false;
                continue;
            }

            current_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            assert(current_node_index != NULL_NODE_INDEX);
            if (nodes_[current_node_index].IsTerminal()) {
                uint32_t word_index = nodes_[current_node_index].word_index;
                assert(word_index < words_lengths_.size());

                if constexpr (IsExactWordsMatching) {
                    size_t word_length = words_lengths_[word_index];
                    size_t l = i + 1 - word_length;
                    if ((l == 0 || !IsInAlphabet(text[l - 1])) && (i + 1 == text.size() || !IsInAlphabet(text[i + 1]))) {
                        find_callback(current_line, word_index);
                    }
                }
                else {
                    find_callback(current_line, word_index);
                }
            }

            // Jump up through compressed suffix links
            for (uint32_t tmp_node_index = nodes_[current_node_index].compressed_suffix_link;
                tmp_node_index != ROOT_INDEX;
                tmp_node_index = nodes_[tmp_node_index].compressed_suffix_link) {
                assert(tmp_node_index != NULL_NODE_INDEX && nodes_[tmp_node_index].IsTerminal());
                size_t word_index = nodes_[tmp_node_index].word_index;
                assert(word_index < words_lengths_.size());

                if constexpr (IsExactWordsMatching) {
                    size_t word_length = words_lengths_[word_index];
                    size_t l = i + 1 - word_length;
                    if ((l == 0 || !IsInAlphabet(text[l - 1])) && (i + 1 == text.size() || !IsInAlphabet(text[i + 1]))) {
                        find_callback(current_line, word_index);
                    }
                }
                else {
                    find_callback(current_line, word_index);
                }
            }

            prev_symbol_in_alphabet = true;
        }

        return current_line;
    }

private:
    static inline constexpr size_t CharToEdgeIndex(char c) noexcept {
        return static_cast<size_t>(static_cast<uint8_t>(c)) - ALPHABET_START;
    }

    static inline constexpr size_t CharToEdgeIndex(int c) noexcept {
        return static_cast<size_t>(static_cast<uint8_t>(c)) - ALPHABET_START;
    }

    static inline constexpr bool IsInAlphabet(char c) noexcept {
        return static_cast<uint32_t>(c) - ALPHABET_START <= ALPHABET_END - ALPHABET_START;
    }

    static_assert(IsInAlphabet(static_cast<char>(ALPHABET_START)));
    static_assert(!IsInAlphabet(static_cast<char>(ALPHABET_START - 1)));
    static_assert(IsInAlphabet(static_cast<char>(ALPHABET_END)));
    static_assert(!IsInAlphabet(static_cast<char>(ALPHABET_END + 1)));

    static inline constexpr bool IsInAlphabet(int c) noexcept {
        return static_cast<uint32_t>(c) - ALPHABET_START <= ALPHABET_END - ALPHABET_START;
    }

    static_assert(IsInAlphabet(static_cast<int>(ALPHABET_START)));
    static_assert(!IsInAlphabet(static_cast<int>(ALPHABET_START - 1)));
    static_assert(IsInAlphabet(static_cast<int>(ALPHABET_END)));
    static_assert(!IsInAlphabet(static_cast<int>(ALPHABET_END + 1)));

    static inline constexpr bool IsUpper(char c) noexcept {
        return static_cast<uint32_t>(c) - 'A' <= 'Z' - 'A';
    }

    static inline constexpr char ToLower(char c) noexcept {
        // return static_cast<char>(c | ('a' - 'A') * IsUpper(c));
        static_assert('a' - 'A' == (1 << 5));
        return static_cast<char>(c | (IsUpper(c) << 5));
    }

    static_assert(ToLower('\0') == '\0');
    static_assert(ToLower('a') == 'a');
    static_assert(ToLower('z') == 'z');
    static_assert(ToLower('A') == 'a');
    static_assert(ToLower('Z') == 'z');
    static_assert(ToLower('~') == '~');

    static inline constexpr bool IsUpper(int c) noexcept {
        return static_cast<uint32_t>(c) - 'A' <= 'Z' - 'A';
    }

    static inline constexpr int ToLower(int c) noexcept {
        // return c | ('a' - 'A') * IsUpper(c);
        static_assert('a' - 'A' == (1 << 5));
        return c | (IsUpper(c) << 5);
    }

    static_assert(ToLower(0) == 0);
    static_assert(ToLower(static_cast<int>('a')) == 'a');
    static_assert(ToLower(static_cast<int>('z')) == 'z');
    static_assert(ToLower(static_cast<int>('A')) == 'a');
    static_assert(ToLower(static_cast<int>('Z')) == 'z');
    static_assert(ToLower(static_cast<int>('~')) == '~');

#ifndef NDEBUG
    inline void CheckComputedLinks() const {
        typename std::vector<ACTNode>::const_iterator iter = nodes_.begin();

        uint32_t max_node_index_excl = static_cast<uint32_t>(nodes_.size());
        assert(max_node_index_excl >= DEFAULT_NODES_COUNT);
        uint32_t max_word_end_index_excl = static_cast<uint32_t>(words_lengths_.size());

        // Check just for existing
        static_assert(NULL_NODE_INDEX == NULL_NODE_INDEX, "current impl of CheckComputedLinks() relies on NULL_NODE_INDEX");

        // Skip null node
        ++iter;
        // Now iter points to fake preroot node
        // fake preroot node does not have suffix_link_index and compressed_suffix_link
        // all children point to root
        for (uint32_t child_index : iter->edges) {
            assert(child_index == ROOT_INDEX);
        }

        ++iter;
        // Now iter points to the root node
        for (auto iter_end = nodes_.end(); iter != iter_end; ++iter) {
            static_assert(NULL_NODE_INDEX < FAKE_PREROOT_INDEX);

            for (uint32_t child_index : iter->edges) {
                assert(child_index >= FAKE_PREROOT_INDEX && child_index < max_node_index_excl);
            }

            uint32_t suffix_link_index = iter->suffix_link;
            assert(suffix_link_index >= FAKE_PREROOT_INDEX && suffix_link_index < max_node_index_excl);

            uint32_t compressed_suffix_link_index = iter->compressed_suffix_link;
            assert(compressed_suffix_link_index >= FAKE_PREROOT_INDEX && compressed_suffix_link_index < max_node_index_excl);

            assert(!iter->IsTerminal() || (iter->word_index < max_word_end_index_excl));
        }

        are_links_computed_ = true;
    }
#endif
};

} // namespace ACTrieADS

#endif // _ACTRIE_H_
