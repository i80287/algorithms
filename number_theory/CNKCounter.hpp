#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#define CXX20_CONSTEXPR constexpr
#else
#define CXX20_CONSTEXPR
#endif

class SquareMatrix final {
public:
    using side_type              = typename std::uint32_t;
    using value_type             = typename std::uint64_t;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using size_type              = typename std::size_t;
    using allocator_type         = typename std::allocator<value_type>;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = typename std::reverse_iterator<iterator>;
    using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

    explicit CXX20_CONSTEXPR SquareMatrix(side_type side_size)
        : data_(allocate_square_matrix(side_size)), side_size_(side_size) {
        std::fill(begin(), end(), value_type{0});
    }
    CXX20_CONSTEXPR SquareMatrix(const SquareMatrix& other) : SquareMatrix(other.side_size_) {
        static_assert(std::is_nothrow_default_constructible_v<value_type> &&
                          std::is_nothrow_copy_assignable_v<value_type>,
                      "");
        std::copy(other.begin(), other.end(), begin());
    }
    CXX20_CONSTEXPR SquareMatrix& operator=(const SquareMatrix& other) {
        return *this = SquareMatrix(other);
    }
    CXX20_CONSTEXPR SquareMatrix(SquareMatrix&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          side_size_(std::exchange(other.side_size_, side_type{})) {}
    CXX20_CONSTEXPR SquareMatrix& operator=(SquareMatrix&& other) noexcept {
        this->swap(other);
        return *this;
    }
    CXX20_CONSTEXPR ~SquareMatrix() {
        deallocate_square_matrix(data(), size());
    }
    CXX20_CONSTEXPR void swap(SquareMatrix& other) noexcept {
        // std::swap is not constexpr in the C++17
        std::swap(data_, other.data_);
        std::swap(side_size_, other.side_size_);
    }
    friend CXX20_CONSTEXPR void swap(SquareMatrix& lhs, SquareMatrix& rhs) noexcept {
        lhs.swap(rhs);
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr size_type flat_size() const noexcept {
        return side_size() * side_size();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr size_type size() const noexcept {
        return flat_size();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr size_type side_size() const noexcept {
        return side_size_;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr size_type rows() const noexcept {
        return side_size();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr size_type cols() const noexcept {
        return side_size();
    }
    struct Shape {
        size_type rows;
        size_type cols;
    };
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr Shape shape() const noexcept {
        return {rows(), cols()};
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr pointer data() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_pointer data() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr iterator begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr iterator end() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data() + size();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator begin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator end() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data() + size();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator cbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator cend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr reverse_iterator rbegin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{end()};
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr reverse_iterator rend() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{begin()};
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_reverse_iterator rbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{end()};
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_reverse_iterator rend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{begin()};
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_reverse_iterator crbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rbegin();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr const_reverse_iterator crend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rend();
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE pointer operator[](size_type i) noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_ + i * cols();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE const_pointer
    operator[](size_type i) const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_ + i * cols();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE reference operator()(size_type i,
                                                 size_type j) noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[i * cols() + j];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE value_type operator()(size_type i, size_type j) const noexcept {
        return data_[i * cols() + j];
    }

private:
    static CXX20_CONSTEXPR allocator_type get_allocator() noexcept {
        return allocator_type{};
    }
    static CXX20_CONSTEXPR pointer allocate_square_matrix(side_type side_size) {
        return get_allocator().allocate(size_type{side_size} * size_type{side_size});
    }
    static CXX20_CONSTEXPR void deallocate_square_matrix(pointer data_, size_type size) noexcept {
        get_allocator().deallocate(data_, size);
    }

    pointer data_;
    side_type side_size_;
};

inline constexpr std::uint32_t kNoMod = 0;

template <std::uint32_t Mod = kNoMod>
class CNKCounter {
public:
    using storage          = SquareMatrix;
    using max_precalc_type = typename storage::side_type;
    using size_type        = typename storage::size_type;
    using int_type         = typename storage::value_type;

    static_assert(std::is_unsigned_v<int_type>, "");
    static_assert(Mod == kNoMod || Mod <= std::numeric_limits<int_type>::max(),
                  "Too big Mod value");

    explicit CNKCounter(const max_precalc_type max_cashed_n) : c_n_k_table_(max_cashed_n + 1) {
        for (size_type n = 0; n <= max_cashed_n; ++n) {
            c_n_k_table_(n, 0) = 1;
            c_n_k_table_(n, n) = 1;
            for (size_type k = 1; k < n; ++k) {
                c_n_k_table_(n, k) = ByMod(c_n_k_table_(n - 1, k) + c_n_k_table_(n - 1, k - 1));
            }
        }
    }

    [[nodiscard]] int_type operator()(size_type n, size_type k) const noexcept {
        if (n < k) {
            return 0;
        }
        // C(n, k) = C(n, n - k)
        k = std::min(k, n - k);
        if (precalced_for_n(n)) {
            return c_n_k_table_(n, k);
        }

        switch (k) {
            case 0:
                return ByMod(1);
            case 1:
                return ByMod(n);
            case 2:
                if constexpr (CanMultiplyResiduals()) {
                    return ByMod((ByMod(n) * ByMod(n - 1)) / 2);
                }
                break;
            default:
                break;
        }

        const int_type C_n_1_k_1 = (*this)(n - 1, k - 1);
        const int_type C_n_1_k   = (*this)(n - 1, k);
        return ByMod(C_n_1_k_1 + C_n_1_k);
    }

private:
    storage c_n_k_table_;

    constexpr bool precalced_for_n(int_type n) const noexcept {
        return n < c_n_k_table_.side_size();
    }

    static constexpr bool CanMultiplyResiduals() noexcept {
        if constexpr (Mod != kNoMod) {
            return Mod <= std::numeric_limits<int_type>::max() / Mod;
        } else {
            return true;
        }
    }
    static constexpr int_type ByMod(int_type num) noexcept {
        if constexpr (Mod != kNoMod) {
            return static_cast<int_type>(num % Mod);
        } else {
            return static_cast<int_type>(num);
        }
    }
};