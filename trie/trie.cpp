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

#if __cplusplus >= 202002L
#define cpp20_constexpr constexpr
#else
#define cpp20_constexpr
#endif

template <uint8_t kAlphabetStart, uint8_t kAlphabetEnd, bool kIsCaseInsensetive>
class ACTrieBase {
protected:
    static_assert('\0' < kAlphabetStart && kAlphabetStart < kAlphabetEnd && kAlphabetEnd <= SCHAR_MAX);

    // Default value = 'z' - 'a' + 1 = 26
    static constexpr uint8_t kAlphabetLength = kAlphabetEnd - kAlphabetStart + 1;
    static constexpr size_t kDefaultNodesCapacity = 16;

    static constexpr size_t kNullNodeIndex = 0;
    static constexpr size_t kFakePreRootIndex = 1;
    static constexpr size_t kRootIndex = 2;
    static constexpr size_t kDefaultNodesCount = 3; // null node; fake preroot node; root node

    static_assert(std::max(kNullNodeIndex, std::max(kFakePreRootIndex, kRootIndex)) < kDefaultNodesCount);

    struct ACTNode {
        static constexpr uint32_t kMissingSentiel = static_cast<uint32_t>(-1);

        // Indexes in array of nodes
        uint32_t edges[kAlphabetLength];

        // Index in array of nodes
        uint32_t suffix_link = kNullNodeIndex;

        // Index in array of nodes
        uint32_t compressed_suffix_link = kNullNodeIndex; 

        /* 
        * Index of the word in the ac trie which ends on this
        * kMissingSentiel if node is not terminal
        */
        uint32_t word_index = kMissingSentiel;

        ACTNode() noexcept {
            std::memset(edges, static_cast<int>(kNullNodeIndex), sizeof(edges));
        }

        constexpr bool IsTerminal() const noexcept {
            return word_index != kMissingSentiel;
        }
    };

    std::vector<ACTNode> nodes_;
    std::vector<uint32_t> words_lengths_;
    mutable bool are_links_computed_ = false;

public:
    inline ACTrieBase() : nodes_(kDefaultNodesCount) {
        nodes_.reserve(kDefaultNodesCapacity);

        /*
         * link(root) = fake_vertex;
         * For all chars from the alphabet: fake_vertex ---char--> root
         */
        nodes_[kRootIndex].suffix_link = kFakePreRootIndex;
        nodes_[kRootIndex].compressed_suffix_link = kRootIndex;

        for (uint32_t& edge : nodes_[kFakePreRootIndex].edges) {
            edge = kRootIndex;
        }
    }

    /*
     * Aho–Corasick deterministic finite-state machine is built on the ordinal trie.
     */
    constexpr bool ContainsPattern(std::string_view pattern) const noexcept {
        uint32_t current_node_index = kRootIndex;
        for (char sigma : pattern) {
            if constexpr (kIsCaseInsensetive) {
                sigma = ToLower(sigma);
            }

            if (!IsInAlphabet(sigma)) {
                return false;
            }

            uint32_t next_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            if (next_node_index != kNullNodeIndex) {
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
        bfs_queue.push_back(kRootIndex);

        do {
            uint32_t vertex_index = bfs_queue.front();
            bfs_queue.pop_front();

            // to(v, sigma) === vertex.edges[sigma]
            uint32_t* vertex_edges = nodes_[vertex_index].edges;
            uint32_t vertex_suffix_link = nodes_[vertex_index].suffix_link;

            assert(vertex_suffix_link != kNullNodeIndex);
            // to((link(v), sigma)) === nodes[vertex.suffix_link].edges[sigma]
            const uint32_t* vertex_suffix_link_edges = nodes_[vertex_suffix_link].edges;

            // For each char (sigma) in the Alphabet vertex_edges[sigma] is the child such: v --sigma--> child
            for (size_t sigma = 0; sigma != kAlphabetLength; ++sigma) {
                uint32_t child_link_v_index = vertex_suffix_link_edges[sigma];
                assert(child_link_v_index != kNullNodeIndex);

                // child = to(v, sigma)
                uint32_t child_index = vertex_edges[sigma];

                // to((v, sigma)) = to((v, sigma)) if (v, sigma) in the rng(to) else to((link(v), sigma))
                // rng(to) is a range of function 'to'
                if (child_index != kNullNodeIndex) {
                    // link(to(v, sigma)) = to((link(v), sigma)) if (v, sigma) in the rng(to)
                    nodes_[child_index].suffix_link = child_link_v_index;

                    assert(nodes_[child_link_v_index].compressed_suffix_link != kNullNodeIndex);

                    // comp(v) = link(v) if link(v) is terminal or root else comp(link(v))
                    nodes_[child_index].compressed_suffix_link =
                        ((!nodes_[child_link_v_index].IsTerminal()) & (child_link_v_index != kRootIndex))
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

    cpp20_constexpr size_t NodesSize() const noexcept {
        return nodes_.size();
    }

    cpp20_constexpr size_t PatternsSize() const noexcept {
        return words_lengths_.size();
    }

    template <typename FindCallback>
#if __cplusplus >= 202002L
    requires requires (FindCallback func, std::string_view found_word, size_t start_index_in_original_text) {
        func(found_word, start_index_in_original_text);
    }
#endif
    void RunText(std::string_view text, FindCallback find_callback) const {
        assert(IsReady());

        if constexpr (std::is_pointer_v<decltype(find_callback)>) {
            assert(find_callback != nullptr);
        }

        uint32_t current_node_index = kRootIndex;
        size_t i = 0;
        for (auto iter = text.begin(), end = text.end(); iter != end; ++iter, ++i) {
            int32_t sigma = int32_t(uint8_t(*iter));
            if constexpr (kIsCaseInsensetive) {
                sigma = ToLower(sigma);
            }

            if (!IsInAlphabet(sigma)) {
                current_node_index = kRootIndex;
                continue;
            }

            current_node_index = nodes_[current_node_index].edges[CharToEdgeIndex(sigma)];
            assert(current_node_index != kNullNodeIndex);
            if (nodes_[current_node_index].IsTerminal()) {
                uint32_t word_index = nodes_[current_node_index].word_index;
                assert(word_index < words_lengths_.size());
                size_t word_length = words_lengths_[word_index];
                size_t l = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }

            // Jump up through compressed suffix links
            for (uint32_t tmp_node_index = nodes_[current_node_index].compressed_suffix_link;
                tmp_node_index != kRootIndex;
                tmp_node_index = nodes_[tmp_node_index].compressed_suffix_link) {
                assert(tmp_node_index != kNullNodeIndex && nodes_[tmp_node_index].IsTerminal());
                size_t word_index = nodes_[tmp_node_index].word_index;
                assert(word_index < words_lengths_.size());
                size_t word_length = words_lengths_[word_index];
                size_t l = i + 1 - word_length;
                find_callback(text.substr(l, word_length), l);
            }
        }
    }

protected:
    static constexpr size_t CharToEdgeIndex(char c) noexcept {
        return static_cast<size_t>(static_cast<uint8_t>(c)) - kAlphabetStart;
    }

    static constexpr size_t CharToEdgeIndex(int32_t c) noexcept {
        return static_cast<size_t>(c) - kAlphabetStart;
    }

    static constexpr bool IsInAlphabet(char c) noexcept {
        return static_cast<uint32_t>(static_cast<uint8_t>(c)) - kAlphabetStart <= kAlphabetEnd - kAlphabetStart;
    }

    static_assert(IsInAlphabet(static_cast<char>(kAlphabetStart)));
    static_assert(!IsInAlphabet(static_cast<char>(kAlphabetStart - 1)));
    static_assert(IsInAlphabet(static_cast<char>(kAlphabetEnd)));
    static_assert(!IsInAlphabet(static_cast<char>(kAlphabetEnd + 1)));

    static constexpr bool IsInAlphabet(int32_t c) noexcept {
        return static_cast<uint32_t>(c) - kAlphabetStart <= kAlphabetEnd - kAlphabetStart;
    }

    static_assert(IsInAlphabet(static_cast<int32_t>(kAlphabetStart)));
    static_assert(!IsInAlphabet(static_cast<int32_t>(kAlphabetStart - 1)));
    static_assert(IsInAlphabet(static_cast<int32_t>(kAlphabetEnd)));
    static_assert(!IsInAlphabet(static_cast<int32_t>(kAlphabetEnd + 1)));

    static constexpr bool IsUpper(char c) noexcept {
        return static_cast<uint32_t>(c) - 'A' <= 'Z' - 'A';
    }

    static constexpr char ToLower(char c) noexcept {
        if constexpr ((('a' - 'A') & ('a' - 'A' - 1)) == 0) {
            return static_cast<char>(uint8_t(c) | IsUpper(c) * ('a' - 'A'));
        }
        else {
            return static_cast<char>(uint8_t(c) | (-(uint32_t)IsUpper(c) & ('a' - 'A')));
        }
    }

#if defined(__GNUC__) && !defined(__clang__)
    static_assert(ToLower('\0') == '\0');
    static_assert(ToLower('a') == 'a');
    static_assert(ToLower('z') == 'z');
    static_assert(ToLower('A') == 'a');
    static_assert(ToLower('Z') == 'z');
    static_assert(ToLower('~') == '~');
#endif

    static constexpr bool IsUpper(int32_t c) noexcept {
        return static_cast<uint32_t>(c) - 'A' <= 'Z' - 'A';
    }

    static constexpr int32_t ToLower(int32_t c) noexcept {
        if constexpr ((('a' - 'A') & ('a' - 'A' - 1)) == 0) {
            return c | int32_t(('a' - 'A') * IsUpper(c));
        }
        else {
            return c | int32_t(('a' - 'A') & -(uint32_t)IsUpper(c));
        }
    }

    static_assert(ToLower(static_cast<int32_t>('\0')) == 0);
    static_assert(ToLower(static_cast<int32_t>('a')) == 'a');
    static_assert(ToLower(static_cast<int32_t>('z')) == 'z');
    static_assert(ToLower(static_cast<int32_t>('A')) == 'a');
    static_assert(ToLower(static_cast<int32_t>('Z')) == 'z');
    static_assert(ToLower(static_cast<int32_t>('~')) == '~');

#ifndef NDEBUG
    inline void CheckComputedLinks() const {
        auto iter = nodes_.begin();

        uint32_t max_node_index_excluding = static_cast<uint32_t>(nodes_.size());
        assert(max_node_index_excluding >= kDefaultNodesCount);
        uint32_t max_word_end_index_excl = static_cast<uint32_t>(words_lengths_.size());

        static_assert(kNullNodeIndex == kNullNodeIndex, "Current impl of CheckComputedLinks() relies on kNullNodeIndex");
        // Skip null node
        ++iter;
        // Now iter points on fake preroot node
        // fake preroot node does not have suffix_link_index and compressed_suffix_link
        // all children point to root
        for (uint32_t child_index : iter->edges) {
            assert(child_index == kRootIndex);
        }

        ++iter;
        // Now iter points on the root node
        for (auto iter_end = nodes_.end(); iter != iter_end; ++iter) {
            static_assert(kNullNodeIndex < kFakePreRootIndex);

            for (uint32_t child_index : iter->edges) {
                assert(child_index >= kFakePreRootIndex && child_index < max_node_index_excluding);
            }

            uint32_t suffix_link_index = iter->suffix_link;
            assert(suffix_link_index >= kFakePreRootIndex && suffix_link_index < max_node_index_excluding);

            uint32_t compressed_suffix_link_index = iter->compressed_suffix_link;
            assert(compressed_suffix_link_index >= kFakePreRootIndex && compressed_suffix_link_index < max_node_index_excluding);

            assert(!iter->IsTerminal() || (iter->word_index < max_word_end_index_excl));
        }

        are_links_computed_ = true;
    }
#endif
};

template <uint8_t kAlphabetStart = 'A', uint8_t kAlphabetEnd = 'z', bool kIsCaseInsensetive = false>
class ACTrie final : public ACTrieBase<kAlphabetStart, kAlphabetEnd, kIsCaseInsensetive> {
    using base = ACTrieBase<kAlphabetStart, kAlphabetEnd, kIsCaseInsensetive>;
public:
    cpp20_constexpr void ReservePlaceForPatterns(size_t patterns_capacity) {
        this->words_lengths_.reserve(patterns_capacity);
    }

    void AddPattern(std::string_view pattern) {
        assert(!this->IsReady());

        uint32_t current_node_index = this->kRootIndex;
        const unsigned char* pattern_iter = reinterpret_cast<const unsigned char*>(pattern.begin());
        const unsigned char* pattern_end = reinterpret_cast<const unsigned char*>(pattern.end());

        for (; pattern_iter != pattern_end; ++pattern_iter) {
            int32_t sigma = int32_t(*pattern_iter);
            if constexpr (kIsCaseInsensetive) {
                sigma = base::ToLower(sigma);
            }

            if (!base::IsInAlphabet(sigma)) {
                assert(false && "ACTrie::AddPattern(std::string_view): char in pattern is not in the alphabet");
                continue;
            }

            uint32_t next_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)];
            if (next_node_index != base::kNullNodeIndex) {
                current_node_index = next_node_index;
            }
            else {
                break;
            }
        }

        size_t lasted_max_length = static_cast<size_t>(pattern_end - pattern_iter);
        this->nodes_.reserve(this->nodes_.size() + lasted_max_length);

        /*
         * Inserts substring [i..length - 1] of pattern if i < length (<=> i != length)
         * If i == length, then for cycle will no execute
         */
        for (; pattern_iter != pattern_end; ++pattern_iter) {
            int32_t sigma = int32_t(*pattern_iter);
            if constexpr (kIsCaseInsensetive) {
                sigma = base::ToLower(sigma);
            }

            if (!base::IsInAlphabet(sigma)) {
                assert(false && "char in pattern is not in alphabet!!!");
                continue;
            }

            uint32_t new_node_index = static_cast<uint32_t>(this->nodes_.size());
            this->nodes_.emplace_back();
            this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = new_node_index;
            current_node_index = new_node_index;
        }

        uint32_t word_index = static_cast<uint32_t>(this->words_lengths_.size());
        this->nodes_[current_node_index].word_index = word_index;
        this->words_lengths_.push_back(static_cast<uint32_t>(pattern.size()));
    }

    template <char Delimeter = '\n'>
    void ReadPatternsFromCStdin(size_t strings_count) {
        static_assert(!base::IsInAlphabet(Delimeter));
        assert(!base::IsReady());

        ReservePlaceForPatterns(strings_count);

        while (strings_count--) {
            uint32_t string_length = 0;
            uint32_t current_node_index = base::kRootIndex;

            for (int sigma;;) {
                sigma = std::getchar();
                if constexpr (kIsCaseInsensetive) {
                    sigma = base::ToLower(sigma);
                }

                if (!base::IsInAlphabet(sigma)) {
                    if (sigma == Delimeter || sigma == EOF) {
                        break;
                    }

                    assert(false && "char in pattern is not in alphabet!!!");
                    continue;
                }

                string_length++;
                uint32_t next_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)];
                if (next_node_index != base::kNullNodeIndex) {
                    current_node_index = next_node_index;
                }
                else {
                    next_node_index = this->nodes_.size();
                    this->nodes_.emplace_back();
                    this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = next_node_index;
                    current_node_index = next_node_index;

                    while (true) {
                        sigma = std::getchar();
                        if constexpr (kIsCaseInsensetive) {
                            sigma = base::ToLower(sigma);
                        }

                        if (!base::IsInAlphabet(sigma)) {
                            if (sigma == Delimeter || sigma == EOF) {
                                break;
                            }

                            assert(false && "char in pattern is not in alphabet!!!");
                            continue;
                        }

                        string_length++;
                        next_node_index = this->nodes_.size();
                        this->nodes_.emplace_back();
                        this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = next_node_index;
                        current_node_index = next_node_index;
                    }

                    uint32_t word_index = static_cast<uint32_t>(this->words_lengths_.size());
                    this->nodes_[current_node_index].word_index = word_index;
                    this->words_lengths_.push_back(string_length);
                    break;
                }
            }
        }
    }

    template <char Delimeter = '\n'>
    void ReadPatternsFromCppStdin(size_t strings_count) {
        static_assert(!base::IsInAlphabet(Delimeter));
        assert(!base::IsReady());

        ReservePlaceForPatterns(strings_count);

        while (strings_count--) {
            uint32_t string_length = 0;
            uint32_t current_node_index = base::kRootIndex;

            for (int sigma;;) {
                sigma = std::cin.get();
                if constexpr (kIsCaseInsensetive) {
                    sigma = base::ToLower(sigma);
                }

                if (!base::IsInAlphabet(sigma)) {
                    if (sigma == Delimeter || sigma == std::char_traits<char>::eof()) {
                        break;
                    }

                    assert(false && "char in pattern is not in alphabet!!!");
                    continue;
                }

                string_length++;
                uint32_t next_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)];
                if (next_node_index != base::kNullNodeIndex) {
                    current_node_index = next_node_index;
                }
                else {
                    next_node_index = static_cast<uint32_t>(this->nodes_.size());
                    this->nodes_.emplace_back();
                    this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = next_node_index;
                    current_node_index = next_node_index;

                    while (true) {
                        sigma = std::cin.get();
                        if constexpr (kIsCaseInsensetive) {
                            sigma = base::ToLower(sigma);
                        }

                        if (!base::IsInAlphabet(sigma)) {
                            if ((sigma == Delimeter) | (sigma == std::char_traits<char>::eof())) {
                                break;
                            }

                            assert(false && "char in pattern is not in alphabet!!!");
                            continue;
                        }

                        if (!base::IsInAlphabet(sigma)) {
                            assert(false && "char in pattern is not in alphabet!!!");
                            continue;
                        }

                        string_length++;
                        next_node_index = static_cast<uint32_t>(this->nodes_.size());
                        this->nodes_.emplace_back();
                        this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = next_node_index;
                        current_node_index = next_node_index;
                    }

                    uint32_t word_index = static_cast<uint32_t>(this->words_lengths_.size());
                    this->nodes_[current_node_index].word_index = word_index;
                    this->words_lengths_.push_back(string_length);
                    break;
                }
            }
        }
    }
};

template <uint8_t kAlphabetStart = 'A', uint8_t kAlphabetEnd = 'z', bool kIsCaseInsensetive = false>
class ReplacingACTrie final : public ACTrieBase<kAlphabetStart, kAlphabetEnd, kIsCaseInsensetive> {
    using base = ACTrieBase<kAlphabetStart, kAlphabetEnd, kIsCaseInsensetive>;
    std::vector<std::string> words_replacements_;
public:
    cpp20_constexpr void ReservePlaceForPatterns(size_t patterns_capacity) {
        this->words_lengths_.reserve(patterns_capacity);
        words_replacements_.reserve(patterns_capacity);
    }

    void AddPatternWithReplacements(std::string_view pattern, std::string_view replacement) {
        assert(!this->IsReady());

        uint32_t current_node_index = this->kRootIndex;
        const unsigned char* pattern_iter = reinterpret_cast<const unsigned char*>(pattern.begin());
        const unsigned char* pattern_end = reinterpret_cast<const unsigned char*>(pattern.end());

        for (; pattern_iter != pattern_end; ++pattern_iter) {
            int32_t sigma = int32_t(*pattern_iter);
            if constexpr (kIsCaseInsensetive) {
                sigma = base::ToLower(sigma);
            }

            if (!base::IsInAlphabet(sigma)) {
                assert(false && "char in pattern is not in alphabet!!!");
                continue;
            }

            uint32_t next_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)];
            if (next_node_index != base::kNullNodeIndex) {
                current_node_index = next_node_index;
            }
            else {
                break;
            }
        }

        size_t lasted_max_length = static_cast<size_t>(pattern_end - pattern_iter);
        this->nodes_.reserve(this->nodes_.size() + lasted_max_length);

        /*
         * Inserts substring [i..length - 1] of pattern if i < length (<=> i != length)
         * If i == length, then for cycle will no execute
         */
        for (; pattern_iter != pattern_end; ++pattern_iter) {
            int32_t sigma = int32_t(*pattern_iter);
            if constexpr (kIsCaseInsensetive) {
                sigma = base::ToLower(sigma);
            }

            if (!base::IsInAlphabet(sigma)) {
                assert(false && "char in pattern is not in alphabet!!!");
                continue;
            }

            uint32_t new_node_index = static_cast<uint32_t>(this->nodes_.size());
            this->nodes_.emplace_back();
            this->nodes_[current_node_index].edges[base::CharToEdgeIndex(sigma)] = new_node_index;
            current_node_index = new_node_index;
        }

        uint32_t word_index = static_cast<uint32_t>(this->words_lengths_.size());
        this->nodes_[current_node_index].word_index = word_index;
        this->words_lengths_.push_back(static_cast<uint32_t>(pattern.size()));
        words_replacements_.emplace_back(replacement);
    }

    void ReplaceAllOccurances(std::string& text) const {
        assert(this->IsReady());

        struct replacement_info_t {
            uint32_t word_l_index_in_text;
            uint32_t word_index;
        };

        std::vector<replacement_info_t> stack;
        size_t length = text.size();
        size_t new_length = length;
        uint32_t current_node_index = base::kRootIndex;
        for (char* c_string = text.data(), * current_c_string = c_string, * iter = current_c_string; ; ++iter) {
            int32_t c = int32_t(uint8_t(*iter));
            if (c == '\0') {
                break;
            }

            if constexpr (kIsCaseInsensetive) {
                c = base::ToLower(c);
            }

            if (!base::IsInAlphabet(c)) {
                current_node_index = base::kRootIndex;
                continue;
            }

            current_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(c)];
            assert(current_node_index != base::kNullNodeIndex);

            const typename base::ACTNode& current_node = this->nodes_[current_node_index];
            bool current_node_is_terminal = current_node.IsTerminal();
            size_t compressed_suffix_link = current_node.compressed_suffix_link;

            if (current_node_is_terminal | (compressed_suffix_link != base::kRootIndex)) {
                assert(current_node_is_terminal || this->nodes_[compressed_suffix_link].IsTerminal());
                uint32_t word_index = current_node_is_terminal ? current_node.word_index : this->nodes_[compressed_suffix_link].word_index;

                assert(word_index < this->words_lengths_.size());
                uint32_t r_index_in_current_substring = static_cast<uint32_t>(iter - current_c_string);
                uint32_t word_length = this->words_lengths_[word_index];
                // Matched pattern is current_c_string[l; r]
                uint32_t l_index_in_current_substring = r_index_in_current_substring + 1 - word_length;
                size_t replacement_length = words_replacements_[word_index].size();

                if (stack.empty() & (word_length == replacement_length)) {
                    char* dst_address = current_c_string + l_index_in_current_substring;
                    assert(c_string <= dst_address && dst_address + word_length <= c_string + length);
                    memcpy(dst_address, words_replacements_[word_index].c_str(), word_length);
                }
                else {
                    uint32_t word_l_index_in_text = static_cast<uint32_t>(current_c_string + l_index_in_current_substring - c_string);
                    stack.emplace_back(word_l_index_in_text, word_index);
                    new_length += (replacement_length - word_length);
                }

                current_c_string = iter + 1;
                current_node_index = base::kRootIndex;
            }
        }

        if (length == new_length) {
            return;
        }
        size_t right_offset = 0;
        if (new_length > length) {
            text.resize(new_length);
        }

        char* c_string = text.data();
        for (const replacement_info_t* stack_iter_end = stack.data() - 1, * stack_iter = stack_iter_end + stack.size();
            stack_iter != stack_iter_end; --stack_iter) {
            uint32_t word_index = stack_iter->word_index;
            uint32_t word_l_index_in_text = stack_iter->word_l_index_in_text;
            uint32_t word_length = this->words_lengths_[word_index];
            size_t moved_part_length = length - (word_l_index_in_text + word_length);

            char*       dst_address = c_string + (new_length - right_offset - moved_part_length);
            const char* src_address = c_string + word_l_index_in_text + word_length;

            if (dst_address != src_address) {
                // If pattern length != replacement length, worth checking
                assert(c_string <= dst_address && dst_address + moved_part_length <= c_string + std::max(new_length, length));
                assert(c_string <= src_address && src_address + moved_part_length <= c_string + std::max(new_length, length));
                memmove(dst_address, src_address, moved_part_length);            
            }

            size_t replacement_length = words_replacements_[word_index].size();
            dst_address -= replacement_length;

            assert(c_string <= dst_address && dst_address + replacement_length <= c_string + std::max(new_length, length));
            memcpy(dst_address, words_replacements_[word_index].c_str(), replacement_length);

            length = word_l_index_in_text;
            right_offset += moved_part_length + replacement_length;
        }

        if (new_length < text.size()) {
            text.resize(new_length);
        }
    }
    
    void ReplaceFirstOccurance(std::string& text) const {
        uint32_t current_node_index = base::kRootIndex;
        for (const char* iter = text.c_str(); ; ++iter) {
            int32_t c = int32_t(uint8_t(*iter));
            if (c == '\0') {
                break;
            }

            if constexpr (kIsCaseInsensetive) {
                c = base::ToLower(c);
            }

            if (!base::IsInAlphabet(c)) {
                current_node_index = base::kRootIndex;
                continue;
            }

            current_node_index = this->nodes_[current_node_index].edges[base::CharToEdgeIndex(c)];
            assert(current_node_index != base::kNullNodeIndex);
            const typename base::ACTNode& current_node = this->nodes_[current_node_index];
            bool current_node_is_terminal = current_node.IsTerminal();
            size_t compressed_suffix_link = current_node.compressed_suffix_link;

            if (current_node_is_terminal | (compressed_suffix_link != base::kRootIndex)) {
                uint32_t word_index;
                if (current_node_is_terminal) {
                    word_index = current_node.word_index;
                }
                else {
                    assert(this->nodes_[compressed_suffix_link].IsTerminal());
                    word_index = this->nodes_[compressed_suffix_link].word_index;
                }

                assert(word_index < this->words_lengths_.size());
                uint32_t word_length = this->words_lengths_[word_index];
                uint32_t r_index_in_current_substring = static_cast<uint32_t>(iter - text.c_str());

                // Matched pattern is text[l; r]
                uint32_t l = r_index_in_current_substring + 1 - word_length;
                size_t replacement_length = words_replacements_[word_index].size();

                if (replacement_length != word_length) {
                    // In both cases: replacement_length > word_length || replacement_length < word_length code is the same
                    text.resize(text.size() - word_length + replacement_length);
                    memmove(text.data() + l + replacement_length,
                        text.c_str() + r_index_in_current_substring + 1,
                        text.size() - 1 - r_index_in_current_substring);
                }

                memcpy(text.data() + l, words_replacements_[word_index].c_str(), replacement_length);
                break;
            }
        }
    }
};

} // namespace ACTrieADS

namespace actrie_tests {

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

    t.ComputeLinks();
    if (!t.IsReady()) {
        return false;
    }

    std::vector<std::pair<std::string_view, size_t>> found_occurances;
    found_occurances.reserve(expected_occurances.size());
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

}

namespace replacing_actrie_tests {

template <size_t PatternsSize>
void run_test(const char* (&patterns_with_replacements)[PatternsSize][2], std::string& input_text, std::string_view expected, bool replace_all_occurances, uint32_t test_number) {
    ACTrieADS::ReplacingACTrie<'-', '}', true> t;
    t.ReservePlaceForPatterns(PatternsSize);

    for (size_t i = 0; i < PatternsSize; i++) {
        t.AddPatternWithReplacements(patterns_with_replacements[i][0], patterns_with_replacements[i][1]);
    }

    bool result = true;
    if (t.PatternsSize() != PatternsSize) {
        result = false;
        goto cleanup;
    }

    t.ComputeLinks();
    if (!t.IsReady()) {
        result = false;
        goto cleanup;
    }

    for (size_t i = 0; i < PatternsSize; i++) {
        if (!t.ContainsPattern(patterns_with_replacements[i][0])) {
            result = false;
            goto cleanup;
        }
    }

    if (replace_all_occurances) {
        t.ReplaceAllOccurances(input_text);
    }
    else {
        t.ReplaceFirstOccurance(input_text);
    }
    result &= input_text == expected;
cleanup:
    printf((result ? "test %u passed\n" : "test %u not passed\n"), test_number);
}

[[maybe_unused]] static void test0() {
    const char* patterns_with_replacements[][2] = {
        { "ab", "cd" },
        { "ba", "dc" },
        { "aa", "cc" },
        { "bb", "dd" },
        { "fasb", "xfasbx" },
    };
    std::string input_text = "ababcdacafaasbfasbabcc";
    const char exptected[] = "cdcdcdacafccsbxfasbxcdcc";
    run_test(patterns_with_replacements, input_text, exptected, true, 0);
}

[[maybe_unused]] static void test1() {
    const char* patterns_with_replacements[][2] = {
        {"ab", "cd"},
        {"ba", "dc"},
        {"aa", "cc"},
        {"bb", "dd"},
        {"xfasbx", "fasb"},
    };
    std::string input_text = "ababcdacafaasbxfasbxabcc";
    const char exptected[] = "cdcdcdacafccsbfasbcdcc";
    run_test(patterns_with_replacements, input_text, exptected, true, 1);
}

[[maybe_unused]] static void test2() {
    const char* patterns_with_replacements[][2] = {
        {"LM", "0000"},
        {"GHI", "111111"},
        {"BCD", "2222222"},
        {"nop", "3333"},
        {"jk", "44444"}
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const char exptected[] = "A2222222EF1111114444400003333";
    run_test(patterns_with_replacements, input_text, exptected, true, 2);
}

[[maybe_unused]] static void test3() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "111111111111111111111111"},
        {"CD", "cd"},
        {"EF", "ef"},
        {"JK", "jk"},
        {"NO", "no"}
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const char expected[]  = "111111111111111111111111cdefGHIjkLMnoP";
    run_test(patterns_with_replacements, input_text, expected, true, 3);
}

[[maybe_unused]] static void test4() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "ab"},
        {"CD", "cd"},
        {"EF", "ef"},
        {"JK", "jk"},
        {"NO", "111111111111111111111111"}
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const char expected[]  = "abcdefGHIjkLM111111111111111111111111P";
    run_test(patterns_with_replacements, input_text, expected, true, 4);
}

[[maybe_unused]] static void test5() {
    const char* patterns_with_replacements[][2] = {
        {"AB", "ab"},
        {"CD", "cd"},
        {"EF", "111111111111111111111111"},
        {"JK", "jk"},
        {"NO", "no"}
    };
    std::string input_text = "ABCDEFGHIJKLMNOP";
    const char expected[]  = "abcd111111111111111111111111GHIjkLMnoP";
    run_test(patterns_with_replacements, input_text, expected, true, 5);
}

[[maybe_unused]] static void test6() {
    const char* patterns_with_replacements[][2] = {
        {"kernel", "Kewnel"},
        {"linux", "Linuwu"},
        {"debian", "Debinyan"},
        {"ubuntu", "Uwuntu"},
        {"windows", "WinyandOwOws"}
    };
    std::string input_text = "linux kernel; debian os; ubuntu os; windows os";
    const char expected[]  = "Linuwu Kewnel; Debinyan os; Uwuntu os; WinyandOwOws os";
    run_test(patterns_with_replacements, input_text, expected, true, 6);
}

[[maybe_unused]] static void test7() {
    const char* patterns_with_replacements[][2] = {
        {"brew-cask", "bwew-cawsk"},
        {"brew-cellar", "bwew-cewwaw"},
        {"emerge", "emewge"},
        {"flatpak", "fwatpakkies"},
        {"pacman", "pacnyan"},
        {"port", "powt"},
        {"rpm", "rawrpm"},
        {"snap", "snyap"},
        {"zypper", "zyppew"},

        {"lenovo", "LenOwO"},
        {"cpu", "CPUwU"},
        {"core", "Cowe"},
        {"gpu", "GPUwU"},
        {"graphics", "Gwaphics"},
        {"corporation", "COwOpowation"},
        {"nvidia", "NyaVIDIA"},
        {"mobile", "Mwobile"},
        {"intel", "Inteww"},
        {"radeon", "Radenyan"},
        {"geforce", "GeFOwOce"},
        {"raspberry", "Nyasberry"},
        {"broadcom", "Bwoadcom"},
        {"motorola", "MotOwOwa"},
        {"proliant", "ProLinyant"},
        {"poweredge", "POwOwEdge"},
        {"apple", "Nyapple"},
        {"electronic", "ElectrOwOnic"},
        {"processor", "Pwocessow"},
        {"microsoft", "MicOwOsoft"},
        {"ryzen", "Wyzen"},
        {"advanced", "Adwanced"},
        {"micro", "Micwo"},
        {"devices", "Dewices"},
        {"inc.", "Nyanc."},
        {"lucienne", "Lucienyan"},
        {"tuxedo", "TUWUXEDO"},
        {"aura", "Uwura"},

        {"linux", "linuwu"},
        {"alpine", "Nyalpine"},
        {"amogos", "AmogOwOS"},
        {"android", "Nyandroid"},
        {"arch", "Nyarch Linuwu"},

        {"arcolinux", "ArcOwO Linuwu"},

        {"artix", "Nyartix Linuwu"},
        {"debian", "Debinyan"},

        {"devuan", "Devunyan"},

        {"deepin", "Dewepyn"},
        {"endeavouros", "endeavOwO"},
        {"fedora", "Fedowa"},
        {"femboyos", "FemboyOWOS"},
        {"gentoo", "GentOwO"},
        {"gnu", "gnUwU"},
        {"guix", "gnUwU gUwUix"},
        {"linuxmint", "LinUWU Miwint"},
        {"manjaro", "Myanjawo"},
        {"manjaro-arm", "Myanjawo AWM"},
        {"neon", "KDE NeOwOn"},
        {"nixos", "nixOwOs"},
        {"opensuse-leap", "OwOpenSUSE Leap"},
        {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
        {"pop", "PopOwOS"},
        {"raspbian", "RaspNyan"},
        {"rocky", "Wocky Linuwu"},
        {"slackware", "Swackwawe"},
        {"solus", "sOwOlus"},
        {"ubuntu", "Uwuntu"},
        {"void", "OwOid"},
        {"xerolinux", "xuwulinux"},

        // BSD
        {"freebsd", "FweeBSD"},
        {"openbsd", "OwOpenBSD"},

        // Apple family
        {"macos", "macOwOS"},
        {"ios", "iOwOS"},

        // Windows
        {"windows", "WinyandOwOws"},
    };
    std::string input_text = "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel processor";
    const char expected[]  = "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    run_test(patterns_with_replacements, input_text, expected, true, 7);
}

[[maybe_unused]] static void test8() {
    const char* patterns_with_replacements[][2] = {
        {"brew-cask", "bwew-cawsk"},
        {"brew-cellar", "bwew-cewwaw"},
        {"emerge", "emewge"},
        {"flatpak", "fwatpakkies"},
        {"pacman", "pacnyan"},
        {"port", "powt"},
        {"rpm", "rawrpm"},
        {"snap", "snyap"},
        {"zypper", "zyppew"},

        {"lenovo", "LenOwO"},
        {"cpu", "CPUwU"},
        {"core", "Cowe"},
        {"gpu", "GPUwU"},
        {"graphics", "Gwaphics"},
        {"corporation", "COwOpowation"},
        {"nvidia", "NyaVIDIA"},
        {"mobile", "Mwobile"},
        {"intel", "Inteww"},
        {"radeon", "Radenyan"},
        {"geforce", "GeFOwOce"},
        {"raspberry", "Nyasberry"},
        {"broadcom", "Bwoadcom"},
        {"motorola", "MotOwOwa"},
        {"proliant", "ProLinyant"},
        {"poweredge", "POwOwEdge"},
        {"apple", "Nyapple"},
        {"electronic", "ElectrOwOnic"},
        {"processor", "Pwocessow"},
        {"microsoft", "MicOwOsoft"},
        {"ryzen", "Wyzen"},
        {"advanced", "Adwanced"},
        {"micro", "Micwo"},
        {"devices", "Dewices"},
        {"inc.", "Nyanc."},
        {"lucienne", "Lucienyan"},
        {"tuxedo", "TUWUXEDO"},
        {"aura", "Uwura"},

        {"linux", "linuwu"},
        {"alpine", "Nyalpine"},
        {"amogos", "AmogOwOS"},
        {"android", "Nyandroid"},
        {"arch", "Nyarch Linuwu"},

        {"arcolinux", "ArcOwO Linuwu"},

        {"artix", "Nyartix Linuwu"},
        {"debian", "Debinyan"},

        {"devuan", "Devunyan"},

        {"deepin", "Dewepyn"},
        {"endeavouros", "endeavOwO"},
        {"fedora", "Fedowa"},
        {"femboyos", "FemboyOWOS"},
        {"gentoo", "GentOwO"},
        {"gnu", "gnUwU"},
        {"guix", "gnUwU gUwUix"},
        {"linuxmint", "LinUWU Miwint"},
        {"manjaro", "Myanjawo"},
        {"manjaro-arm", "Myanjawo AWM"},
        {"neon", "KDE NeOwOn"},
        {"nixos", "nixOwOs"},
        {"opensuse-leap", "OwOpenSUSE Leap"},
        {"opensuse-tumbleweed", "OwOpenSUSE Tumbleweed"},
        {"pop", "PopOwOS"},
        {"raspbian", "RaspNyan"},
        {"rocky", "Wocky Linuwu"},
        {"slackware", "Swackwawe"},
        {"solus", "sOwOlus"},
        {"ubuntu", "Uwuntu"},
        {"void", "OwOid"},
        {"xerolinux", "xuwulinux"},

        // BSD
        {"freebsd", "FweeBSD"},
        {"openbsd", "OwOpenBSD"},

        // Apple family
        {"macos", "macOwOS"},
        {"ios", "iOwOS"},

        // Windows
        {"windows", "WinyandOwOws"},
    };
    std::string input_text = "windows freebsd rocky; neon linux; fedora; pop os; solus; amogos; void; ryzen and intel processor";
    const char expected[]  = "WinyandOwOws FweeBSD Wocky Linuwu; KDE NeOwOn linuwu; Fedowa; PopOwOS os; sOwOlus; AmogOwOS; OwOid; Wyzen and Inteww Pwocessow";
    run_test(patterns_with_replacements, input_text, expected, true, 8);
}

[[maybe_unused]] static void test9() {
    const char* patterns_with_replacements[][2] = {
        { "abc", "def" },
        { "ghi", "jkz" },
    };
    std::string input_text = "Abghciashjdhwdjahwdjhabdabanabwc";
    const char expected[]  = "Abghciashjdhwdjahwdjhabdabanabwc";
    run_test(patterns_with_replacements, input_text, expected, true, 9);
}

[[maybe_unused]] static void test10() {
    const char* patterns_with_replacements[][2] = {
        { "abc", "def" },
        { "ghi", "jkz" },
    };
    std::string input_text = "Qghiabcabcghiabc";
    const char expected[]  = "Qjkzabcabcghiabc";
    run_test(patterns_with_replacements, input_text, expected, false, 10);
}

}

int main() {
    actrie_tests::test0();
    actrie_tests::test1();
    actrie_tests::test2();

    replacing_actrie_tests::test0();
    replacing_actrie_tests::test1();
    replacing_actrie_tests::test2();
    replacing_actrie_tests::test3();
    replacing_actrie_tests::test4();
    replacing_actrie_tests::test5();
    replacing_actrie_tests::test6();
    replacing_actrie_tests::test7();
    replacing_actrie_tests::test8();
    replacing_actrie_tests::test9();
    replacing_actrie_tests::test10();
    return 0;
}
