#include <cassert>
#include <cstdint>
#include <new>
#include <vector>
#include <cstdlib>

#if __cplusplus >= 202002L
#include <bit>
#endif

template <class value_t = int64_t>
struct SparseTable {
    constexpr SparseTable() noexcept = default;

    SparseTable(const std::vector<value_t>& data) : SparseTable(data.data(), data.size()) {}

    SparseTable(const value_t* data, size_t n) {
        const size_t row_len = log2_floored(n) + 1;

        size_t table_with_double_index_size =
            align_size(n * sizeof(value_t*) + (n * row_len) * sizeof(value_t));
        size_t log_table_size = align_size((n + 1) * sizeof(size_t));
        void* raw_bytes = operator new(table_with_double_index_size + log_table_size);

        table = static_cast<value_t**>(raw_bytes);
        floored_log_table =
            std::launder(reinterpret_cast<size_t*>(static_cast<std::byte*>(raw_bytes) + table_with_double_index_size));

        value_t* table_data_row = std::launder(reinterpret_cast<value_t*>(table + n));
        for (size_t i = 0; i < n; i++) {
            table[i]    = table_data_row;
            table[i][0] = data[i];
            table_data_row += row_len;
        }

        for (size_t j = 1; j < row_len; j++) {
            size_t jmp = 1u << (j - 1);
            size_t i   = 0;
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

    SparseTable(const SparseTable& other)            = delete;
    SparseTable& operator=(const SparseTable& other) = delete;

    constexpr SparseTable(SparseTable&& other) noexcept
        : table(other.table), floored_log_table(other.floored_log_table) {
        other.table             = nullptr;
        other.floored_log_table = nullptr;
    }

    SparseTable& operator=(SparseTable&& other) noexcept {
        auto t1                 = other.table;
        auto t2                 = other.floored_log_table;
        other.table             = nullptr;
        other.floored_log_table = nullptr;
        this->~SparseTable();
        table             = t1;
        floored_log_table = t2;
        return *this;
    }

    ~SparseTable() {
        operator delete(table);
    }

    value_t operator()(size_t l, size_t r) const noexcept {
        assert(l <= r);
        size_t j = floored_log_table[r - l + 1];
        return std::min(table[l][j], table[r - (1u << j) + 1][j]);
    }

private:
    static constexpr size_t log2_floored(size_t n) noexcept {
#if __cplusplus >= 202002L
        const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
        const uint32_t lz_count = uint32_t(__builtin_clzll(n | 1));
#endif
        return 63 ^ lz_count;
    }

    static constexpr size_t align_size(size_t size) noexcept {
        return (size + 15) & ~size_t(15);
    }

    value_t** table           = nullptr;
    size_t* floored_log_table = nullptr;
};
