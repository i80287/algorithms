#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <vector>

#include "../../number_theory/math_functions.hpp"

template <class value_t = int64_t>
class SparseTable {
public:
    static_assert(std::is_arithmetic_v<value_t>);

    using size_type = std::size_t;

    explicit SparseTable(const std::vector<value_t>& data) : SparseTable(data.data(), data.size()) {}

    SparseTable(const value_t* data, size_type n) {
        assert(n > 0);
        const size_type row_len = math_functions::log2_floor(n | 1) + 1;

        size_type table_with_double_index_size = align_size(n * sizeof(value_t*) + (n * row_len) * sizeof(value_t));
        size_type log_table_size = align_size((n + 1) * sizeof(size_type));
        void* raw_bytes = ::operator new(table_with_double_index_size + log_table_size);

        table_ = static_cast<value_t**>(raw_bytes);
        floored_log_table_ = std::launder(
            reinterpret_cast<size_type*>(static_cast<std::byte*>(raw_bytes) + table_with_double_index_size));

        value_t* table_data_row = std::launder(reinterpret_cast<value_t*>(table_ + n));
        for (size_type i = 0; i < n; i++) {
            table_[i] = table_data_row;
            table_[i][0] = data[i];
            table_data_row += row_len;
        }

        for (size_type j = 1; j < row_len; j++) {
            size_type jmp = 1u << (j - 1);
            size_type i = 0;
            for (; i + jmp < n; i++) {
                table_[i][j] = std::min(table_[i][j - 1], table_[i + jmp][j - 1]);
            }
            for (; i < n; i++) {
                table_[i][j] = table_[i][j - 1];
            }
        }

        floored_log_table_[0] = static_cast<size_type>(-1);
        for (size_type i = 1; i <= n; i++) {
            floored_log_table_[i] = floored_log_table_[i / 2] + 1;
        }
    }

    SparseTable(const SparseTable& other) = delete;
    SparseTable& operator=(const SparseTable& other) = delete;

    constexpr SparseTable(SparseTable&& other) noexcept
        : table_(other.table_), floored_log_table_(other.floored_log_table_) {
        other.table_ = nullptr;
        other.floored_log_table_ = nullptr;
    }

    SparseTable& operator=(SparseTable&& other) noexcept {
        SparseTable{std::move(other)}.swap(*this);
        return *this;
    }

    ~SparseTable() {
        ::operator delete(table_);
    }

    [[nodiscard]] value_t operator()(const size_type l, const size_type r) const noexcept {
        assert(l <= r);
        const size_type j = floored_log_table_[r - l + 1];
        return std::min(table_[l][j], table_[r - (1u << j) + 1][j]);
    }

    void swap(SparseTable& other) noexcept {
        std::swap(table_, other.table_);
        std::swap(floored_log_table_, other.floored_log_table_);
    }

    friend void swap(SparseTable& lhs, SparseTable& rhs) noexcept {
        lhs.swap(rhs);
    }

private:
    [[nodiscard]] static constexpr size_type align_size(const size_type size) noexcept {
        return (size + 15) & ~size_type{15};
    }

    value_t** table_ = nullptr;
    size_type* floored_log_table_ = nullptr;
};
