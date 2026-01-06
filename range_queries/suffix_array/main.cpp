#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using std::cin, std::cout;
using std::string;
using std::vector;

#if __cplusplus >= 202002L
#include <bit>
#endif

static constexpr bool is_2_pow(size_t n) noexcept {
    return (n & (n - 1)) == 0;
}

static constexpr size_t log2_floored(size_t n) noexcept {
#if __cplusplus >= 202002L
    const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
    const uint32_t lz_count = uint32_t(__builtin_clzll(n | 1));
#endif
    return 63 ^ lz_count;
}

static constexpr size_t log2_ceiled(size_t n) noexcept {
    return log2_floored(n) + !is_2_pow(n);
}

struct SuffixArray {
    vector<size_t> p;
    vector<size_t> c;

    SuffixArray(const std::string& s) : p(s.size()), c(s.size()) {
        /**
         *
         * Example on the first iteration, k = 0, len = 1 << k = 1
         *
         * 7: "#abacaba"
         * 6: "a#abacab"
         * 4: "aba#abac"
         * 2: "acaba#ab"
         * 0: "abacaba#"
         * 5: "ba#abaca"
         * 1: "bacaba#a"
         * 3: "caba#aba"
         *
         * p: { 7, 6, 4, 2, 0, 5, 1, 3 }
         *
         * c is applied to the original string
         * c: { 1, 2, 1, 3, 1, 2, 1, 0 }
         */

        const size_t n = s.size();

        // suffix array
        vector<size_t> p_prev(n);
        // chars equivalence classes array
        vector<size_t> c_prev(n);

        vector<size_t> cnt(std::max(n, size_t(CHAR_MAX + 1)));
        for (size_t i = 0; i < n; i++) {
            cnt[uint8_t(s[i])]++;
        }
        for (size_t chr = 1; chr <= CHAR_MAX; chr++) {
            cnt[chr] += cnt[chr - 1];
        }
        for (size_t i = 0; i < n; i++) {
            p[--cnt[uint8_t(s[i])]] = i;
        }
        c[p[0]] = 0;
        for (size_t i = 1; i < n; i++) {
            c[p[i]] = c[p[i - 1]] + (s[p[i]] != s[p[i - 1]]);
        }

        for (size_t k = 1, logncl = log2_ceiled(n); k <= logncl; k++) {
            p.swap(p_prev);
            c.swap(c_prev);

            // Sort p by right part
            for (size_t i = 0; i < n; i++) {
                p_prev[i] = (p_prev[i] + n - (1u << (k - 1))) % n;
            }

            // Stable sort p by left part
            cnt.assign(n, 0);
            for (size_t i = 0; i < n; i++) {
                cnt[c_prev[i]]++;  /// <=> cnt[c_prev[p_prev[i]]]++;
            }
            for (size_t i = 1; i < n; i++) {
                cnt[i] += cnt[i - 1];
            }
            for (size_t i = n - 1; int64_t(i) >= 0; i--) {
                p[--cnt[c_prev[p_prev[i]]]] = p_prev[i];
            }

            c[p[0]] = 0;
            for (size_t i = 1; i < n; i++) {
                size_t p_i_1 = p[i - 1];
                size_t p_i = p[i];
                size_t shift = 1u << (k - 1);
                size_t i2 = (p_i_1 + shift) % n;
                size_t j2 = (p_i + shift) % n;
                c[p[i]] = c[p[i - 1]] + (c_prev[p_i_1] != c_prev[p_i] || c_prev[i2] != c_prev[j2]);
            }
        }

        assert(c[n - 1] == 0);
        for (size_t i = 0; i < n; i++) {
            assert(p[c[i]] == i);
        }
    }
};

template <class value_t = int64_t>
struct SparseTable {
    value_t** table = nullptr;
    size_t* floored_log_table = nullptr;

    constexpr SparseTable() noexcept = default;

    SparseTable(const vector<value_t>& data) : SparseTable(data.data(), data.size()) {}

    SparseTable(const value_t* data, size_t n) {
        const size_t row_len = log2_floored(n) + 1;

        size_t table_with_double_index_size = n * sizeof(value_t*) + (n * row_len) * sizeof(value_t);
        size_t log_table_size = (n + 1) * sizeof(size_t);
        char* raw_bytes = static_cast<char*>(operator new(table_with_double_index_size + log_table_size));

        table = reinterpret_cast<value_t**>(raw_bytes);
        floored_log_table = reinterpret_cast<size_t*>(raw_bytes + table_with_double_index_size);

        value_t* table_data_row = reinterpret_cast<value_t*>(table + n);
        for (size_t i = 0; i < n; i++) {
            table[i] = table_data_row;
            table[i][0] = data[i];
            table_data_row += row_len;
        }

        for (size_t j = 1; j < row_len; j++) {
            size_t jmp = 1u << (j - 1);
            size_t i = 0;
            for (; i + jmp < n; i++) {
                table[i][j] = std::min(table[i][j - 1], table[i + jmp][j - 1]);
            }
            for (; i < n; i++) {
                table[i][j] = table[i][j - 1];
            }
        }

        floored_log_table[0] = size_t(-1);
        for (size_t i = 1; i <= n; i++) {
            floored_log_table[i] = floored_log_table[i / 2] + 1;
        }
    }

    SparseTable(const SparseTable& other) = delete;
    SparseTable& operator=(const SparseTable& other) = delete;

    SparseTable(SparseTable&& other) noexcept {
        *this = std::move(other);
    }

    SparseTable& operator=(SparseTable&& other) noexcept {
        auto t1 = other.table;
        auto t2 = other.floored_log_table;
        other.table = nullptr;
        other.floored_log_table = nullptr;
        this->~SparseTable();
        table = t1;
        floored_log_table = t2;
        return *this;
    }

    ~SparseTable() {
        operator delete(table);
    }

    value_t operator()(size_t l, size_t r) noexcept {
        assert(l <= r);
        size_t j = floored_log_table[r - l + 1];
        return std::min(table[l][j], table[r - (1u << j) + 1][j]);
    }
};

struct LCP {
    SuffixArray sa;
    // lcp[i] = LCP(p[i], p[i + 1])
    vector<size_t> lcp;
    SparseTable<size_t> sparse_table;

    LCP(const std::string& s) : sa(s), lcp(s.size() - 1) {
        const size_t n = s.size();
        for (size_t k = 0, i = 0; i + 1 < n; i++) {
            if (k > 0) {
                k--;
            }
            while (s[(i + k) % n] == s[(sa.p[sa.c[i] - 1] + k) % n]) {
                k++;
            }
            lcp[sa.c[i] - 1] = k;
        }

        sparse_table = SparseTable(lcp);
    }

    size_t operator()(size_t i, size_t j) noexcept {
        if (i >= j) {
            if (__builtin_expect(i == j, false)) {
                // string length
                return lcp.size() + 1;
            }
            std::swap(i, j);
        }

        return sparse_table(i, j - 1);
    }
};

int main() {
    string s = "abacaba#";
    /**
     * Suffixes:
     * p[0]: #abacaba
     * p[1]: a#abacab
     * p[2]: aba#abac
     * p[3]: abacaba#
     * p[4]: acaba#ab
     * p[5]: ba#abaca
     * p[6]: bacaba#
     * p[7]: caba#aba
     */
    LCP lcp(s);
    assert(lcp(0, 2) == 0);
    assert(lcp(2, 3) == 3);
    assert(lcp(5, 6) == 2);
    assert(lcp(1, 4) == 1);
}
