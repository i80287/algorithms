#include <cassert>     // assert
#include <climits>     // CHAR_MAX
#include <cstdint>     // std::uint32_t, std::size_t
#include <cstdio>      // std::getchar
#include <cstring>     // std::memset
#include <deque>       // std::deque
#include <iostream>    // std::cin, std::cout
#include <string>      // std::string
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace ACTrieADS {

template <typename callback>
concept ACTrieFindCallback = requires (callback func, std::string_view found_word, size_t start_index_in_original_text) {
    func(found_word, start_index_in_original_text);
};

template <uint8_t ALPHABET_START = 'A', uint8_t ALPHABET_END = 'z', bool IsCaseInsensetive = false>
class ACTrie final {
    static_assert('\0' < ALPHABET_START && ALPHABET_START < ALPHABET_END && ALPHABET_END < CHAR_MAX);

    // Default value = 'z' - 'a' + 1 = 26
    static constexpr uint8_t ALPHABET_LENGTH = ALPHABET_END - ALPHABET_START + 1;
    static constexpr size_t DEFAULT_VECTORS_CAPACITY = 16;

    static constexpr size_t NULL_NODE_INDEX = 0;
    static constexpr size_t FAKE_PREROOT_INDEX = 1;
    static constexpr size_t ROOT_INDEX = 2;
    static constexpr size_t DEFAULT_NODES_COUNT = 3; // null node; fake preroot node; root node

    static_assert(std::max(NULL_NODE_INDEX, std::max(FAKE_PREROOT_INDEX, ROOT_INDEX)) < DEFAULT_NODES_COUNT);

    struct ACTNode {
        static constexpr uint32_t MISSING_SENTIEL = static_cast<uint32_t>(-1);

        // Indexes in array of nodes
        uint32_t edges[ALPHABET_LENGTH];

        // Index in array of nodes
        uint32_t suffix_link;

        // Index in array of nodes
        uint32_t compressed_suffix_link; 

        /* 
        * Index of the word in the ac trie which ends on this
        * MISSING_SENTIEL if node is not terminal
        */
        uint32_t word_index;

        constexpr ACTNode() noexcept
            : suffix_link{NULL_NODE_INDEX},
            compressed_suffix_link{NULL_NODE_INDEX},
            word_index{MISSING_SENTIEL} {
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
    constexpr ACTrie() : nodes_(DEFAULT_NODES_COUNT), words_lengths_() {
        nodes_.reserve(DEFAULT_VECTORS_CAPACITY);
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

    template <char Delimeter = '\n'>
    void ReadPatternsFromCStdin(size_t strings_count) {
        static_assert(!IsInAlphabet(Delimeter));
        assert(!IsReady());

        ReservePlaceForPatterns(strings_count);

        while (strings_count--) {
            uint32_t string_length = 0;
            uint32_t current_node_index = ROOT_INDEX;

            int sigma;
            while (true) {
                if constexpr (IsCaseInsensetive) {
                    sigma = ToLower(std::getchar());
                }
                else {
                    sigma = std::getchar();
                }

                if (!IsInAlphabet(sigma)) {
                    if ((sigma == Delimeter) | (sigma == EOF)) {
                        break;
                    }

                    assert(!"char in pattern is not in alphabet!!!");
                    continue;
                }

                string_length++;
                uint32_t next_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
                if (next_node_index != NULL_NODE_INDEX) {
                    current_node_index = next_node_index;
                }
                else {
                    next_node_index = nodes_.size();
                    nodes_.emplace_back();
                    nodes_[current_node_index].edges[CharToEdgeIndex(sigma)] = next_node_index;
                    current_node_index = next_node_index;

                    while (true) {
                        if constexpr (IsCaseInsensetive) {
                            sigma = ToLower(std::getchar());
                        }
                        else {
                            sigma = std::getchar();
                        }

                        if (!IsInAlphabet(sigma)) {
                            if ((sigma == Delimeter) | (sigma == EOF)) {
                                break;
                            }

                            assert(!"char in pattern is not in alphabet!!!");
                            continue;
                        }

                        string_length++;
                        next_node_index = nodes_.size();
                        nodes_.emplace_back();
                        nodes_[current_node_index].edges[CharToEdgeIndex(sigma)] = next_node_index;
                        current_node_index = next_node_index;
                    }

                    uint32_t word_index = static_cast<uint32_t>(words_lengths_.size());
                    nodes_[current_node_index].word_index = word_index;
                    words_lengths_.push_back(string_length);
                    break;
                }
            }
        }
    }

    template <char Delimeter = '\n'>
    void ReadPatternsFromCppStdin(size_t strings_count) {
        static_assert(!IsInAlphabet(Delimeter));
        assert(!IsReady());

        ReservePlaceForPatterns(strings_count);

        while (strings_count--) {
            uint32_t string_length = 0;
            uint32_t current_node_index = ROOT_INDEX;

            int sigma;
            while (true) {
                if constexpr (IsCaseInsensetive) {
                    sigma = ToLower(std::cin.get());
                }
                else {
                    sigma = std::cin.get();
                }

                if (!IsInAlphabet(sigma)) {
                    if ((sigma == Delimeter) | (sigma == std::char_traits<char>::eof())) {
                        break;
                    }

                    assert(!"char in pattern is not in alphabet!!!");
                    continue;
                }

                string_length++;
                uint32_t next_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
                if (next_node_index != NULL_NODE_INDEX) {
                    current_node_index = next_node_index;
                }
                else {
                    next_node_index = static_cast<uint32_t>(nodes_.size());
                    nodes_.emplace_back();
                    nodes_[current_node_index].edges[CharToEdgeIndex(sigma)] = next_node_index;
                    current_node_index = next_node_index;

                    while (true) {
                        if constexpr (IsCaseInsensetive) {
                            sigma = ToLower(std::cin.get());
                        }
                        else {
                            sigma = std::cin.get();
                        }

                        if (!IsInAlphabet(sigma)) {
                            if ((sigma == Delimeter) | (sigma == std::char_traits<char>::eof())) {
                                break;
                            }

                            assert(!"char in pattern is not in alphabet!!!");
                            continue;
                        }

                        if (!IsInAlphabet(sigma)) {
                            assert(!"char in pattern is not in alphabet!!!");
                            continue;
                        }

                        string_length++;
                        next_node_index = static_cast<uint32_t>(nodes_.size());
                        nodes_.emplace_back();
                        nodes_[current_node_index].edges[CharToEdgeIndex(sigma)] = next_node_index;
                        current_node_index = next_node_index;
                    }

                    uint32_t word_index = static_cast<uint32_t>(words_lengths_.size());
                    nodes_[current_node_index].word_index = word_index;
                    words_lengths_.push_back(string_length);
                    break;
                }
            }
        }
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
    requires ACTrieFindCallback<FindCallback>
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
        return static_cast<char>(c | ('a' - 'A') * IsUpper(c));
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
        return c | ('a' - 'A') * IsUpper(c);
    }

    static_assert(ToLower(0) == 0);
    static_assert(ToLower(static_cast<int>('a')) == 'a');
    static_assert(ToLower(static_cast<int>('z')) == 'z');
    static_assert(ToLower(static_cast<int>('A')) == 'a');
    static_assert(ToLower(static_cast<int>('Z')) == 'z');
    static_assert(ToLower(static_cast<int>('~')) == '~');

#ifndef NDEBUG
    inline void CheckComputedLinks() const {
        auto iter = nodes_.begin();

        uint32_t max_node_index_excl = static_cast<uint32_t>(nodes_.size());
        assert(max_node_index_excl >= DEFAULT_NODES_COUNT);
        uint32_t max_word_end_index_excl = static_cast<uint32_t>(words_lengths_.size());

        static_assert(NULL_NODE_INDEX == NULL_NODE_INDEX, "current impl of CheckComputedLinks() relies on NULL_NODE_INDEX");
        // Skip null node
        ++iter;
        // Now iter points on fake preroot node
        // fake preroot node does not have suffix_link_index and compressed_suffix_link
        // all children point to root
        for (uint32_t child_index : iter->edges) {
            assert(child_index == ROOT_INDEX);
        }

        ++iter;
        // Now iter points on the root node
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


template <size_t PatternsSize>
bool run_tests(const char* (&patterns)[PatternsSize], const char* text, const std::vector<std::pair<std::string_view, size_t>>& expected_occurances) {
    ACTrieADS::ACTrie t;
    for (const char* pattern : patterns) {
        t.AddPattern(pattern);
    }

    for (const char* pattern : patterns) {
        if (!t.ContainsPattern(pattern)) {
            return false;
        }
    }

    if (t.PatternsSize() != PatternsSize) {
        return false;
    }

    std::vector<std::pair<std::string_view, size_t>> found_occurances;
    found_occurances.reserve(expected_occurances.size());

    t.ComputeLinks();
    if (!t.IsReady()) {
        return false;
    }

    t.RunText(text,
        [&found_occurances](std::string_view found_word, size_t start_index_in_original_text) {
            found_occurances.emplace_back(found_word, start_index_in_original_text);
        });

    return found_occurances == expected_occurances;
}

void test0() {
    const char* patterns[] = {
        "a",
        "ab",
        "ba",
        "aa",
        "bb",
        "fasb"
    };

    const char text[] = "ababcdacafaasbfasbabcc";

    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        { "a", 0 },
        { "ab", 0 },
        { "ba", 1 },
        { "a", 2 },
        { "ab", 2 },
        { "a", 6 },
        { "a", 8 },
        { "a", 10 },
        { "aa", 10 },
        { "a", 11 },
        { "a", 15 },
        { "fasb", 14 },
        { "ba", 17 },
        { "a", 18 },
        { "ab", 18 },
    };

    std::cout << (run_tests(patterns, text, expected_occurances) ? "Test 0 passed\n" : "Test 0 not passed\n");
}

void test1() {
    const char* patterns[] = {
        "ABC",
        "CDE",
        "CDEF"
    };

    const char text[] = "ABCDEFGHABCDEFGADCVABCDEBACBCBABDBEBCBABABBCDEBCBABDEBCABDBCBACABCDBEBACBCDEWBCBABCDE";

    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        { "ABC", 0 },
        { "CDE", 2 },
        { "CDEF", 2 },
        { "ABC", 8 },
        { "CDE", 10 },
        { "CDEF", 10 },
        { "ABC", 19 },
        { "CDE", 21 },
        { "CDE", 43 },
        { "ABC", 63 },
        { "CDE", 73 },
        { "ABC", 80 },
        { "CDE", 82 },
    };

    std::cout << (run_tests(patterns, text, expected_occurances) ? "Test 1 passed\n" : "Test 1 not passed\n");
}

void test2() {
    const char* patterns[] = {
        "aba",
        "baca",
        "abacaba",
        "ccbba",
        "cabaaba"
    };

    const char text[] = 
        "ababcbbacbcabaabaacbacbbacbbabcbabcbcabaabaabcabaabacabaabacbabbbacbabacbabbacbcabacabcbcbacabaababcbabbacacbbcbcababbcbcbacabcabacbcababacababcbabccaababacabcbabcbacbabcabcbbababacaababababcbbcbcbcbcbcbababcbabcabccbbcbcbcabaabacabbacbabca";

    std::vector<std::pair<std::string_view, size_t>> expected_occurances = {
        { "aba", 0 },
        { "aba", 11 },
        { "cabaaba", 10 },
        { "aba", 14 },
        { "aba", 37 },
        { "cabaaba", 36 },
        { "aba", 40 },
        { "aba", 46 },
        { "cabaaba", 45 },
        { "aba", 49 },
        { "baca", 50 },
        { "abacaba", 49 },
        { "aba", 53 },
        { "cabaaba", 52 },
        { "aba", 56 },
        { "aba", 68 },
        { "aba", 80 },
        { "baca", 81 },
        { "baca", 89 },
        { "aba", 92 },
        { "cabaaba", 91 },
        { "aba", 95 },
        { "baca", 103 },
        { "aba", 113 },
        { "baca", 121 },
        { "aba", 127 },
        { "aba", 133 },
        { "aba", 135 },
        { "baca", 136 },
        { "abacaba", 135 },
        { "aba", 139 },
        { "aba", 150 },
        { "aba", 152 },
        { "baca", 153 },
        { "aba", 175 },
        { "aba", 177 },
        { "baca", 178 },
        { "aba", 182 },
        { "aba", 184 },
        { "aba", 186 },
        { "aba", 203 },
        { "aba", 223 },
        { "cabaaba", 222 },
        { "aba", 226 },
        { "baca", 227 },
    };

    std::cout << (run_tests(patterns, text, expected_occurances) ? "Test 2 passed\n" : "Test 2 not passed\n");
}

void test3() {
    ACTrieADS::ACTrie<'-', 'z', true> t;
    std::cout << "Input patterns count\n> " << std::flush;
    size_t strings_count = 0;
    std::cin >> strings_count;
    if (std::cin.fail()) {
        std::cout << "An error occured while reading unsigned interger\n";
        return;
    }

    std::cin.get(); // Skip '\n'

    std::cout << "Input " << strings_count << " patterns\n> " << std::flush;
    t.ReadPatternsFromCppStdin(strings_count);
    t.ComputeLinks();

    std::cout << "Input text\n> " << std::flush;
    std::string s;
    std::getline(std::cin, s);
    t.RunText(std::string_view(s),
        [](std::string_view found_word, [[maybe_unused]] size_t start_index_in_original_text) {
            // std::cout << "Word " << found_word << " from " << start_index_in_original_text << " to " << start_index_in_original_text + found_word.size() << '\n';
            std::cout << '\"' << found_word << "\", ";
        });
    t.RunText(std::string_view(s),
        []([[maybe_unused]] std::string_view found_word, size_t start_index_in_original_text) {
            // std::cout << "Word " << found_word << " from " << start_index_in_original_text << " to " << start_index_in_original_text + found_word.size() << '\n';
            std::cout << start_index_in_original_text << ", ";
        });

    std::cout << std::endl;
}

int main(void) {
    //test0();
    //test1();
    //test2();
    test3();
    return 0;
}
