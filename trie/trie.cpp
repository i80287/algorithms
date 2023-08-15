#include <cstdint>
#include <deque>
#include <string>
#include <cstring>
#include <string_view>
#include <vector>
#include <iostream>
#include <set>
#include <cstdio>

namespace ACTrieADS {

#if defined(NDEBUG) && NDEBUG

#define Assert(Expression) ((void)0)
#define AssertRetVal(Expression, RetValue) ((void)0)

#else

#define Assert(Expression)                                              \
do {                                                                    \
    if (!bool(Expression)) {                                            \
        char _assert_buffer[1024] = {};                                 \
        std::sprintf(_assert_buffer,                                    \
            "Assertion failed: %s, file %s, function: %s, line %d\n",   \
            (#Expression),                                              \
            __FILE__,                                                   \
            __PRETTY_FUNCTION__,                                        \
            __LINE__);                                                  \
        std::cerr << _assert_buffer;                                    \
        return;                                                         \
    }                                                                   \
} while (false)                                                         \

#define AssertRetVal(Expression, RetValue)                              \
do {                                                                    \
    if (!bool(Expression)) {                                            \
        char _assert_buffer[1024] = {};                                 \
        std::sprintf(_assert_buffer,                                    \
            "Assertion failed: %s, file %s, function: %s, line %d\n",   \
            (#Expression),                                              \
            __FILE__,                                                   \
            __PRETTY_FUNCTION__,                                        \
            __LINE__);                                                  \
        std::cerr << _assert_buffer;                                    \
    return RetValue;                                                    \
}                                                                       \
} while (false)                                                         \

#endif // NDEBUG


template<uint8_t AlphabetLength>
struct ACTrieNode final {
    static constexpr size_t MISSING_SENTIEL = static_cast<size_t>(-1);

    static_assert(AlphabetLength >= 2);
    ACTrieNode<AlphabetLength>* edges[AlphabetLength];
    ACTrieNode<AlphabetLength>* suffix_link;
    ACTrieNode<AlphabetLength>* compressed_suffix_link;
    size_t word_end_index; // Index of the word in the Trie which ends on this node; -1 if node is not terminal

    constexpr ACTrieNode() noexcept
        : edges{}, suffix_link{nullptr}, compressed_suffix_link{nullptr}, word_end_index{MISSING_SENTIEL} {
    }

    constexpr bool IsTerminal() const noexcept {
        return word_end_index != MISSING_SENTIEL;
    }
};

template <uint8_t AlphabetStart = 'A', uint8_t AlphabetEnd = 'a'>
class ACTrie final {
public:
    static constexpr uint8_t MinAvailableChar = '!';
    static constexpr uint8_t MaxAvailableChar = '~';
    static_assert(MinAvailableChar <= AlphabetStart && AlphabetStart < AlphabetEnd && AlphabetEnd <= MaxAvailableChar);

    // Default value = 'z' - 'a' + 1 = 26
    static constexpr uint8_t AlphabetLength = AlphabetEnd - AlphabetStart + 1;
    using Node = ACTrieNode<AlphabetLength>;
    typedef void (*PrintCallback) (std::string_view, size_t l, size_t r);
    typedef void (*PrintCallbackUnsafe) (const char*, size_t l, size_t r);

private:
    Node m_fake_preroot;
    Node m_root;
    std::vector<size_t> m_words_lengths;

#ifndef NDEBUG
    mutable bool m_is_initialized = false;
#endif

public:
    constexpr ACTrie() noexcept(noexcept(std::vector<size_t>())) : m_fake_preroot{}, m_root{}, m_words_lengths{} {
        /* link(root) = fake_vertex;
         * For all chars from the alphabet: fake_vertex ---char--> root
         */

        Node* root_address= &m_root;
        m_root.suffix_link = &m_fake_preroot;
        m_root.compressed_suffix_link = root_address;

        for (size_t sigma = 0; sigma != AlphabetLength; ++sigma) {
            m_fake_preroot.edges[sigma] = root_address;
        }
    }

    void AddPattern(std::string_view pattern) {
#ifndef NDEBUG
        Assert(!m_is_initialized);
#endif
        Node* current_node = &m_root;

        const char* iter = pattern.begin();
        const char* const end = pattern.end();
        for (; iter != end; ++iter) {
            Assert(AlphabetStart <= *iter && *iter <= AlphabetEnd);

            Node* next_node = current_node->edges[CharToEdge(*iter)];
            if (next_node) {
                current_node = next_node;
            } else {
                break;
            }
        }

        /* Insert substring [i..length - 1] of pattern if i < length (<=> i != length)
         * If i == length, then for cycle will no execute
         */
        for (; iter != end; ++iter) {
            Assert(AlphabetStart <= *iter && *iter <= AlphabetEnd);

            Node* new_node = new Node();
            current_node->edges[CharToEdge(*iter)] = new_node;
            current_node = new_node;
        }

        current_node->word_end_index = m_words_lengths.size();
        m_words_lengths.push_back(pattern.length());
    }

    template <char Delimeter = '\n'>
    void ReadPatternsFromCStdin(size_t strings_count) {
        static_assert(!(AlphabetStart <= Delimeter && Delimeter <= AlphabetEnd));
#ifndef NDEBUG
        Assert(!m_is_initialized);
#endif

        Node* const root_address = &m_root;

        while (strings_count--) {
            int sigma = 0;
            size_t string_length = 0;
            Node* current_node = root_address;

            while ((sigma = getchar()) != Delimeter && sigma != EOF) {
                Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
                ++string_length;
                Node* next_node = current_node->edges[CharToEdge(sigma)];

                if (next_node) {
                    current_node = next_node;
                } else {

                    next_node = new Node();
                    current_node->edges[CharToEdge(sigma)] = next_node;
                    current_node = next_node;

                    while ((sigma = getchar()) != Delimeter) {
                        Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
                        ++string_length;
                        next_node = new Node();
                        current_node->edges[CharToEdge(sigma)] = next_node;
                        current_node = next_node;
                    }

                    current_node->word_end_index = m_words_lengths.size();
                    m_words_lengths.push_back(string_length);

                    break;
                }
            }
        }
    }

    template <char Delimeter = '\n'>
    void ReadPatternsFromCppStdin(size_t strings_count) {
        static_assert(!(AlphabetStart <= Delimeter && Delimeter <= AlphabetEnd));
#ifndef NDEBUG
        Assert(!m_is_initialized);
#endif

        Node* const root_address = &m_root;

        while (strings_count--) {
            int sigma = 0;
            size_t string_length = 0;
            Node* current_node = root_address;

            while ((sigma = std::cin.get()) != Delimeter && sigma != std::char_traits<char>::eof()) {
                Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
                ++string_length;
                Node* next_node = current_node->edges[CharToEdge(sigma)];

                if (next_node) {
                    current_node = next_node;
                } else {

                    next_node = new Node();
                    current_node->edges[CharToEdge(sigma)] = next_node;
                    current_node = next_node;

                    while ((sigma = std::cin.get()) != Delimeter) {
                        Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
                        ++string_length;
                        next_node = new Node();
                        current_node->edges[CharToEdge(sigma)] = next_node;
                        current_node = next_node;
                    }

                    current_node->word_end_index = m_words_lengths.size();
                    m_words_lengths.push_back(string_length);

                    break;
                }
            }
        }
    }

    /*
     * Aho–Corasick deterministic finite-state machine is built on the ordinal trie.
     */
    bool Contains(std::string_view pattern) const noexcept {
        const Node* current_node = &this->m_root;

        for (auto sigma : pattern) {
            AssertRetVal(AlphabetStart <= sigma && sigma <= AlphabetEnd, false);

            const Node* next = current_node->edges[CharToEdge(sigma)];
            if (next) {
                current_node = next;
            } else {
                return false;
            }
        }

        return current_node->m_is_terminal;
    }

    void ComputeLinks() {
#ifndef NDEBUG
        Assert(!m_is_initialized);
#endif
        /* See MIPT lecture https://youtu.be/MEFrIcGsw1o for more info, nice lecture recorder!
         * The base of mathematical induction for word with length == 1 (first row of nodes)
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

        Node* root_address = &m_root;

        /* This is implemented on the first iteration of the BFS.
         * Node** root_edges = m_root.edges;
         * for (size_t sigma = 0; sigma != AlphabetLength; ++sigma) {
         *     if (root_edges[sigma]) {
         *         root_edges[sigma]->suffix_link = root_address;
         *         root_edges[sigma]->compressed_suffix_link = root_address;
         *     } else {
         *         root_edges[sigma] = root_address;
         *     }
         * }
		 */

        // Run BFS through all nodes.
        std::deque<Node*> queue;
        queue.push_back(root_address);
        size_t queue_size = 1;

        do {
            Node* vertex = queue.front();
            // to(v, sigma) <=> vertex->edges[sigma]
            Node** vertex_edges = vertex->edges;
            
            Assert(vertex->suffix_link != nullptr);
            // to((link(v), sigma)) <=> vertex->suffix_link->edges[sigma]
            Node** vertex_suffix_link_edges = vertex->suffix_link->edges;

            queue.pop_front();
            --queue_size;

            // For each char (sigma) in the Alphabet vertex_edges[sigma] is the child such: v --sigma--> child
            for (size_t sigma = 0; sigma != AlphabetLength; ++sigma) {
                Assert(vertex_suffix_link_edges[sigma] != nullptr);

                // child = to(v, sigma)
                Node* child = vertex_edges[sigma];

                // to((v, sigma)) = to((v, sigma)) if (v, sigma) in the rng(to) else to((link(v), sigma))
                if (child) {
                    // link(to(v, sigma)) = to((link(v), sigma)) if (v, sigma) in the rng(to)
                    Node* child_link_v = vertex_suffix_link_edges[sigma];
                    child->suffix_link = child_link_v;

                    Assert(child_link_v->compressed_suffix_link != nullptr);
                    // comp(v) = link(v) if link(v) is terminal or root else comp(link(v))
                    child->compressed_suffix_link =
                        ((!child_link_v->IsTerminal()) & (child_link_v != root_address))
                        ? child_link_v->compressed_suffix_link
                        : child_link_v;

                    queue.push_back(child);
                    ++queue_size;   
                } else {
                    vertex_edges[sigma] = vertex_suffix_link_edges[sigma];
                }
            }
        } while (queue_size != 0);
#ifndef NDEBUG
        CheckComputedLinks();
#endif
    }

    void RunText(std::string_view text, PrintCallback print_callback) const {
        Assert(print_callback != nullptr);
#ifndef NDEBUG
        Assert(m_is_initialized);
#endif

        const Node* const root_address = &m_root;
        const Node* current_node = root_address;

        for (size_t i = 0, length = text.length(); i != length; ++i) {
            Assert(AlphabetStart <= text[i] && text[i] <= AlphabetEnd);
            current_node = current_node->edges[CharToEdge(text[i])];
            Assert(current_node != nullptr);
            if (current_node->IsTerminal()) {
                Assert(current_node->word_end_index < m_words_lengths.size());
                size_t word_length = m_words_lengths[current_node->word_end_index];
                size_t l = i + 1 - word_length;
                print_callback(text.substr(l, word_length), l, i);
            }

            for (const Node* tmp_node = current_node->compressed_suffix_link; tmp_node != root_address; tmp_node = tmp_node->compressed_suffix_link) {
                Assert(tmp_node != nullptr);
                Assert(tmp_node->IsTerminal());
                Assert(tmp_node->word_end_index < m_words_lengths.size());

                size_t word_length = m_words_lengths[tmp_node->word_end_index];
                size_t l = i + 1 - word_length;
                print_callback(text.substr(l, word_length), l, i);
            }
        }
    }

    template <char EndChar = '\n'>
    void ReadAndRunTextFromCStdin(char* buffer, PrintCallbackUnsafe print_callback) const {
        Assert(print_callback != nullptr);
#ifndef NDEBUG
        Assert(m_is_initialized);
#endif

        const Node* const root_address = &m_root;
        const Node* current_node = root_address;
        Assert(buffer != nullptr);
        int sigma = 0;
        for (size_t i = 0; (sigma = getchar()) != EndChar && sigma != EOF; ++i) {
            Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
            buffer[i] = static_cast<char>(sigma);

            current_node = current_node->edges[CharToEdge(sigma)];
            Assert(current_node != nullptr);
            if (current_node->IsTerminal()) {
                size_t l = i + 1 - m_words_lengths[current_node->word_end_index];
                char tmp_char = buffer[i + 1];
                buffer[i + 1] = '\0';
                print_callback(buffer + l, l, i);
                buffer[i + 1] = tmp_char;
            }

            // Jump through compressed suffix links
            for (const Node* tmp_node = current_node->compressed_suffix_link; tmp_node != root_address; tmp_node = tmp_node->compressed_suffix_link) {
                Assert(tmp_node != nullptr);
                Assert(tmp_node->IsTerminal());

                size_t l = i + 1 - m_words_lengths[tmp_node->word_end_index];
                char tmp_char = buffer[i + 1];
                buffer[i + 1] = '\0';
                print_callback(buffer + l, l, i);
                buffer[i + 1] = tmp_char;
            }
        }
    }

    template <char EndChar = '\n'>
    void ReadAndRunTextFromCPPStdin(char* buffer, PrintCallbackUnsafe print_callback) const {
        Assert(print_callback != nullptr);
#ifndef NDEBUG
        Assert(m_is_initialized);
#endif

        const Node* const root_address = &m_root;
        const Node* current_node = root_address;
        Assert(buffer != nullptr);
        int sigma = 0;
        for (size_t i = 0; (sigma = std::cin.get()) != EndChar && sigma != std::char_traits<char>::eof(); ++i) {
            Assert(AlphabetStart <= sigma && sigma <= AlphabetEnd);
            buffer[i] = static_cast<char>(sigma);

            current_node = current_node->edges[CharToEdge(sigma)];
            Assert(current_node != nullptr);
            if (current_node->IsTerminal()) {
                size_t l = i + 1 - m_words_lengths[current_node->word_end_index];
                char tmp_char = buffer[i + 1];
                buffer[i + 1] = '\0';
                print_callback(buffer + l, l, i);
                buffer[i + 1] = tmp_char;
            }

            // Jump through compressed suffix links
            for (const Node* tmp_node = current_node->compressed_suffix_link; tmp_node != root_address; tmp_node = tmp_node->compressed_suffix_link) {
                Assert(tmp_node != nullptr);
                Assert(tmp_node->IsTerminal());

                size_t l = i + 1 - m_words_lengths[tmp_node->word_end_index];
                char tmp_char = buffer[i + 1];
                buffer[i + 1] = '\0';
                print_callback(buffer + l, l, i);
                buffer[i + 1] = tmp_char;
            }
        }
    }

    ~ACTrie() {
        Clear();
    }

    void Clear() try {
        std::deque<const Node*> bfs_queue;
        std::set<const Node*> seen_nodes;
        const Node* const root_address = &m_root;

        {
            const Node* const * vertex_edges = root_address->edges;
            for (size_t i = 0; i != AlphabetLength; ++i) {
                const Node* child = vertex_edges[i];

                if ((child != root_address) & (child != nullptr)) {
                    seen_nodes.insert(child);
                    bfs_queue.push_back(child);
                }
            }
        }

        while (!bfs_queue.empty()) {
            const Node * vertex = bfs_queue.front();
            bfs_queue.pop_front();

            for (const Node* child : vertex->edges) {
                if (((child != root_address) & (child != vertex))
#if __cplusplus > 201703L
                && !seen_nodes.contains(child)
#else
                && seen_nodes.find(child) != seen_nodes.end()
#endif
                && (child != nullptr)) {
                    seen_nodes.insert(child);
                    bfs_queue.push_back(child);
                }
            }

            delete vertex;
        }
    } catch(...) {
    }

private:
    static constexpr size_t CharToEdge(char c) noexcept {
        return static_cast<size_t>((size_t)c - AlphabetStart);
    }

    static constexpr size_t CharToEdge(int c) noexcept {
        return static_cast<size_t>((size_t)c - AlphabetStart);
    }

#ifndef NDEBUG
    void CheckComputedLinks() const try {
        Assert(m_root.suffix_link != nullptr);
        Assert(m_root.compressed_suffix_link != nullptr);

        std::deque<const Node*> bfs_queue;
        std::set<const Node*> seen_nodes;
        const Node* const root_address = &m_root;

        {
            const Node *const *const vertex_edges = root_address->edges;
            for (size_t i = 0; i != AlphabetLength; ++i) {
                const Node *child = vertex_edges[i];
                Assert(child != nullptr);
                if (child != root_address) {
                    seen_nodes.insert(child);
                    bfs_queue.push_back(child);
                }
            }
        }

        while (!bfs_queue.empty()) {
            const Node* vertex = bfs_queue.front();
            Assert(vertex->suffix_link != nullptr);
            Assert(vertex->compressed_suffix_link != nullptr);
            const Node *const *const vertex_edges = vertex->edges;
            bfs_queue.pop_front();

            for (size_t i = 0; i != AlphabetLength; ++i) {
                const Node* child = vertex_edges[i];
                Assert(child != nullptr);
                if (((child != root_address) & (child != vertex))
#if __cplusplus > 201703L
                && !seen_nodes.contains(child)
#else
                && seen_nodes.find(child) != seen_nodes.end()
#endif
                ) {
                    seen_nodes.insert(child);
                    bfs_queue.push_back(child);
                }
            }
        }

        m_is_initialized = true;
    } catch(...) {
        m_is_initialized = true;
    }
#endif
};

} // namespace ACTrieADS

int main(void) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto lambda = [](std::string_view str, size_t l, size_t r) -> void {
        std::cout << "Word " << str << " from " << l << " to " << r << '\n';
    };

    {
        ACTrieADS::ACTrie t;

        t.AddPattern("a");
        t.AddPattern("ab");
        t.AddPattern("ba");
        t.AddPattern("aa");
        t.AddPattern("bb");
        t.AddPattern("fasb");

        t.ComputeLinks();

        t.RunText(
            "ababcdacafaasbfasbabcc",
            lambda
        );
        std::cout << std::endl;
    }

    {
        ACTrieADS::ACTrie<'A', 'Z'> t;
        t.AddPattern("ABC");
        t.AddPattern("CDE");
        t.AddPattern("CDEF");

        t.ComputeLinks();
        std::cout << "Text 'ABCDEFGHABCDEFGADCVABCDEBACBCBABDBEBCBABABBCDEBCBABDEBCABDBCBACABCDBEBACBCDEWBCBABCDE':\n";
        t.RunText(
            "ABCDEFGHABCDEFGADCVABCDEBACBCBABDBEBCBABABBCDEBCBABDEBCABDBCBACABCDBEBACBCDEWBCBABCDE",
            lambda
        );
        std::cout << std::endl;
    }


    {
        ACTrieADS::ACTrie<'a', 'c'> t;
        t.AddPattern("aba");
        t.AddPattern("baca");
        t.AddPattern("abacaba");
        t.AddPattern("ccbba");
        t.AddPattern("cabaaba");

        t.ComputeLinks();
        std::cout << "Text 'ababcbbacbcabaabaacbacbbacbbabcbabcbcabaabaabcabaabacabaabacbabbbacbabacbabbacbcabacabcbcbacabaababcbabbacacbbcbcababbcbcbacabcabacbcababacababcbabccaababacabcbabcbacbabcabcbbababacaababababcbbcbcbcbcbcbababcbabcabccbbcbcbcabaabacabbacbabca':\n";
        t.RunText(
            "ababcbbacbcabaabaacbacbbacbbabcbabcbcabaabaabcabaabacabaabacbabbbacbabacbabbacbcabacabcbcbacabaababcbabbacacbbcbcababbcbcbacabcabacbcababacababcbabccaababacabcbabcbacbabcabcbbababacaababababcbbcbcbcbcbcbababcbabcabccbbcbcbcabaabacabbacbabca",
            lambda
        );
        std::cout << std::endl;
    }

    {
        ACTrieADS::ACTrie<'A', 'z'> t;
        std::cout << "Input 2 patterns\n> " << std::flush;
        t.ReadPatternsFromCppStdin(2);
        t.ComputeLinks();

        std::cout << "Input text\n> " << std::flush;
        if (char* buffer = new(std::nothrow) char[2048]) {
            t.ReadAndRunTextFromCPPStdin(
                buffer,
                [](const char* str, size_t l, size_t r) -> void {
                    std::cout << "Word " << str << "; l = " << l << "; r = " << r << '\n';
                });

            delete[] buffer;
            std::cout << std::endl;
        }
    }

    return 0;
}
