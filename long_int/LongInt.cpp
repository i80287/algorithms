// #define NDEBUG 1

#include <cassert>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <complex>

#include "integers_128_bit.hpp"

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

using f64 = double;
using f128 = long double;
using std::vector;
using std::string;
using std::cin, std::cout;
using complex = std::complex<f64>;

#if __cplusplus >= 202002L
#include <numbers>
inline constexpr f64 PI = std::numbers::pi_v<f64>;
#else
const f64 PI = f64(acosl(-1));
#endif

/*
 * Save only e^{2pi*0/1}, e^{2pi*0/2}, e^{2pi*0/4}, e^{2pi*1/4}, e^{2pi*0/8}, e^{2pi*1/8}, e^{2pi*2/8}, e^{2pi*3/8}, ...
 * because only low n / 2 roots are used (fft_points[0] never used btw, because in fft step >= 1, so it can be anything)
*/
static vector<complex> fft_points = { complex(0, 0), complex(1, 0) };

static inline void CheckRootsCapacity(size_t n) {
    assert((n & (n - 1)) == 0);
    for (size_t current_len = fft_points.size(); current_len < n; current_len *= 2) {
        for (size_t i = current_len / 2; i < current_len; i++) {
            fft_points.emplace_back(fft_points[i]);
            // double phi = 2 * PI * (2 * i - current_len + 1) / (2 * current_len);
            f64 phi = PI * f64(2 * i - current_len + 1) / f64(current_len);
            fft_points.emplace_back(std::cos(phi), std::sin(phi));
        }
    }
}

inline void ForwardFFT(complex* p, const size_t k) noexcept {
    assert(k != 0 && (k & (k - 1)) == 0);
    for (size_t i = 1, j = 0; i < k; i++) {
        size_t bit = k >> 1;
        for (; j >= bit; bit >>= 1) {
            j -= bit;
        }

        j += bit;
        if (i < j) {
            std::swap(p[i], p[j]);
        }
    }

    const complex* points = fft_points.data();
    for (size_t step = 1; step < k; step *= 2) {
        for (size_t block_start = 0; block_start < k; ) {
            size_t point_index = step;
            size_t block_end = block_start + step;
            for (size_t pos_in_block = block_start; pos_in_block < block_end; pos_in_block++, point_index++) {
                complex p0_i = p[pos_in_block];
                auto w_j_p1_i = points[point_index] * p[pos_in_block + step];
                p[pos_in_block] = p0_i + w_j_p1_i;
                p[pos_in_block + step] = p0_i - w_j_p1_i;
            }

            block_start = block_end + step;
        }
    }
}

inline void BackwardFFT(complex* p, const size_t k) noexcept {
    assert(k != 0 && (k & (k - 1)) == 0);
    for (size_t i = 1, j = 0; i < k; i++) {
        size_t bit = k >> 1;
        for (; j >= bit; bit >>= 1) {
            j -= bit;
        }

        j += bit;
        if (i < j) {
            std::swap(p[i], p[j]);
        }
    }

    const complex* points = fft_points.data();
    for (size_t step = 1, point_step = k / 2; step < k; step *= 2, point_step /= 2) {
        for (size_t block_start = 0; block_start < k; ) {
            size_t point_index = step;
            size_t block_end = block_start + step;
            for (size_t pos_in_block = block_start; pos_in_block < block_end; pos_in_block++, point_index++) {
                complex p0_i = p[pos_in_block];
                auto w_j_p1_i =  std::conj(points[point_index]) * p[pos_in_block + step];
                p[pos_in_block] = p0_i + w_j_p1_i;
                p[pos_in_block + step] = p0_i - w_j_p1_i;
            }

            block_start = block_end + step;
        }
    }

    f64 one_kth = 1.0 / f64(k);
    for (complex* p_iter = p, *p_end = p_iter + k; p_iter != p_end; ++p_iter) {
        *p_iter *= one_kth;
    }
}

struct LongInt {
    uint32_t* nums_;
    int32_t size_; // size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
    uint32_t capacity_;

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt() : nums_{nullptr}, size_(0), capacity_(0) {
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt(uint64_t n) : size_(0), capacity_(2) {
        nums_ = static_cast<uint32_t*>(operator new(2 * sizeof(uint32_t)));
        size_ += n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
    }

#if _INTEGERS_128_BIT_
#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt(uint128_t n) : size_(0), capacity_{4} {
        nums_ = static_cast<uint32_t*>(operator new(4 * sizeof(uint32_t)));
        size_ += n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[2] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[3] = uint32_t(n);
    }
#endif

    inline explicit LongInt(std::string_view s) : nums_(nullptr), size_(0), capacity_(0) {
        FromString(s);
    }

    inline LongInt(const LongInt& other) : nums_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ != 0) {
            nums_ = static_cast<uint32_t*>(operator new(capacity_ * sizeof(uint32_t)));
            size_t bsz = USize() * sizeof(uint32_t);
            if (bsz != 0) {
                memcpy(nums_, other.nums_, bsz);
            }
        }
    }

    inline LongInt& operator=(const LongInt& other) {
        operator delete(nums_);
        nums_ = nullptr;
        size_ = other.size_;
        capacity_ = other.capacity_;
        if (capacity_ != 0) {
            nums_ = static_cast<uint32_t*>(operator new(capacity_ * sizeof(uint32_t)));
            size_t bsz = USize() * sizeof(uint32_t);
            if (bsz != 0) {
                memcpy(nums_, other.nums_, bsz);
            }
        }

        return *this;
    }

    inline constexpr LongInt(LongInt&& other) noexcept : nums_(other.nums_), size_(other.size_), capacity_(other.capacity_) {
        other.nums_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(LongInt&& other) noexcept {
        operator delete(nums_);
        nums_ = other.nums_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.nums_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline void Swap(LongInt& other) noexcept {
        std::swap(nums_, other.nums_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(int32_t n) {
        if (capacity_ == 0) {
            nums_ = static_cast<uint32_t*>(operator new(2 * sizeof(uint32_t)));
            capacity_ = 2;
        }
        size_ = int32_t(n > 0) - int32_t(n < 0);
        nums_[0] = uint32_t(n >= 0 ? n : -n);
        return *this;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(uint32_t n) {
        if (capacity_ == 0) {
            nums_ = static_cast<uint32_t*>(operator new(2 * sizeof(uint32_t)));
            capacity_ = 2;
        }

        size_ = n != 0;
        nums_[0] = n;
        return *this;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(int64_t n) {
        int32_t sign = int32_t(n > 0) - int32_t(n < 0);
        *this = uint64_t(n >= 0 ? n : -n);
        size_ *= sign;
        return *this;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(uint64_t n) {
        // size_ = ((64 - std::count_leading_zeros(n)) + 31) / 32;
        if (capacity_ < 2) {
            operator delete(nums_);
            nums_ = static_cast<uint32_t*>(operator new(2 * sizeof(uint32_t)));
            capacity_ = 2;
        }

        nums_[0] = uint32_t(n);
        n >>= 32;
        nums_[1] = uint32_t(n);
        size_ = nums_[1] != 0 ? 2 : nums_[0] != 0;
        return *this;
    }

#if _INTEGERS_128_BIT_
#if __cplusplus >= 202302L
    constexpr
#endif
    inline LongInt& operator=(uint128_t n) {
        size_ = ((128 - std::count_leading_zeros(n)) + 31) / 32;
        if (capacity_ < 4) {
            operator delete(nums_);
            nums_ = static_cast<uint32_t*>(operator new(4 * sizeof(uint32_t)));
            capacity_ = 4;
        }

        nums_[0] = uint32_t(n);
        n >>= 32;
        nums_[1] = uint32_t(n);
        n >>= 32;
        nums_[2] = uint32_t(n);
        n >>= 32;
        nums_[3] = uint32_t(n);
        n >>= 32;
        return *this;
    }
#endif

    LongInt& operator*=(const LongInt& other) {
        size_t k = size_t(uint32_t(std::abs(size_)));
        size_t m = size_t(uint32_t(std::abs(other.size_)));
        uint32_t* ans;
        const uint32_t* k_ptr;
        const uint32_t* m_ptr;

        if (m <= k) {
            k_ptr = nums_;
            m_ptr = other.nums_;
        }
        else {
            k_ptr = other.nums_;
            m_ptr = nums_;
            size_t tmp = m; // let compiler decide wether it is faster the 3 xor
            m = k;
            k = tmp;
        }

        if (unlikely(m == 0)) {
            size_ = 0;
            return *this;
        }

        if ((m <= 16) | (m * k <= 1024)) {
            ans = static_cast<uint32_t*>(operator new((m + k) * sizeof(uint32_t)));
            memset(ans, 0, (m + k) * sizeof(uint32_t));
            for (size_t j = 0; j < m; j++) {
                uint64_t b_j = m_ptr[j];
                uint64_t carry = 0;
                uint64_t carry1 = 0;

                for (size_t i = 0; i < k; i++) {
                    uint64_t a_i = k_ptr[i];
                    uint64_t res = a_i * b_j + carry;
                    carry = res >> 32;
                    uint64_t bit_sum = uint64_t(ans[j + i]) + uint64_t(uint32_t(res)) + carry1;
                    carry1 = bit_sum >> 32;
                    ans[j + i] = uint32_t(bit_sum);
                }

                uint64_t bit_sum = uint64_t(ans[j + k]) + carry + carry1;
                carry1 = bit_sum >> 32;

                // ans[j + n] = uint32_t(bit_sum);
                // ans[j + n + 1] = uint32_t(carry1);
                // But we know that j + 1 == m => carry1 == 0

                // Order is very important if we use this optimization and create
                // ans uint32_t[n + m] instead of ans uint32_t[n + m + 1]
                ans[j + k + (j + 1 != m)] = uint32_t(carry1);
                ans[j + k] = uint32_t(bit_sum);
            }

            capacity_ = uint32_t(m + k);
        }
        else {
            size_t n = 2 * std::nearest_2_pow_greater_equal(m + k);
            complex* p1;
            if (likely(n <= 262144)) {
                complex* p = p1 = static_cast<complex*>(operator new(n * sizeof(complex)));
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p = complex(m_value & 0xffff, k_value & 0xffff);
                    p++;
                    *p = complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p = complex(0, k_value & 0xffff);
                    p++;
                    *p = complex(0, k_value >> 16);
                    p++;
                }
                memset(static_cast<void*>(p), 0, (n - 2 * k) * sizeof(complex));
            }
            else {
                n *= 2;
                complex* p = p1 = static_cast<complex*>(operator new(n * sizeof(complex)));
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                }
                memset(static_cast<void*>(p), 0, (n - 4 * k) * sizeof(complex));
            }

            CheckRootsCapacity(n);
            ForwardFFT(p1, n);
            /*
            * A(w^j) = a_0 + a_1 * w^j + a_2 * w^{2 j} + ... + a_{n - 1} * w^{(n - 1) j}
            * B(w^j) = b_0 + b_1 * w^j + b_2 * w^{2 j} + ... + b_{n - 1} * w^{(n - 1) j}
            * 
            * P = A + B * i = [ A(w^0) + B(w^0) * i, A(w^1) + B(w^1) * i, ... A(w^(n - 1)) + B(w^(n - 1)) * i ]
            * 
            * P(w^j) + conj(P(w^{n - j})) =
            * = A(w^j) + B(w^j) * i + conj(A(w^{n - j}) + B(w^{n - j}) * i) =
            * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + \sum{k=0}{n-1} conj((a_k + b_k * i) * w^{(n - j)k}) =
            * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + \sum{k=0}{n-1} conj((a_k + b_k * i) * w^{-jk}) =
            * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + conj((a_k + b_k * i) * w^{-jk}) =
            * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + conj(a_k + b_k * i) * conj(w^{-jk}) =
            * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + (a_k - b_k * i) * w^{jk} =
            * = \sum{k=0}{n-1} 2 a_k * w^{jk} = 2 * A(w^j)
            * 
            * \implies A(w^j) = (P(w^j) + conj(P(w^{n - j}))) / 2
            * 
            * By analogy it is can be proved that A(w^j) = (P(w^j) - conj(P(w^{n - j}))) / (2 * i)
            * 
            * C(w^j) = A(w^j) * B(w^j) \implies C(w^j) = 
            * = (P(w^j) + conj(P(w^{n - j}))) / 2 * (P(w^j) - conj(P(w^{n - j}))) / (2 * i) =
            * = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j}))) / (4 * i) =
            * = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j}))) / (4 * i) =
            */
            complex* p2 = static_cast<complex*>(operator new(n * sizeof(complex)));
            complex one_quat_i = complex(0, -0.25); // 1 / (4 * i) == -i / 4
            for (size_t j = 0; j < n; j++) {
                size_t n_j = (n - j) & (n - 1); // <=> mod n because n is power of two
                complex p_w_j = p1[j];
                complex p_w_n_j = std::conj(p1[n_j]);
                p2[j] = (p_w_j + p_w_n_j) * (p_w_j - p_w_n_j) * one_quat_i;
            }
            BackwardFFT(p2, n);

            if (likely(n <= 262144)) {
                static_assert(sizeof(uint32_t) <= sizeof(complex));
                complex* p = p2;
                uint64_t carry = 0;
                for (uint32_t* ans_p = reinterpret_cast<uint32_t*>(p1), *ans_p_end = ans_p + (m + k);
                    ans_p != ans_p_end; ++ans_p) {
                    uint64_t res = carry;
                    res += uint64_t((*p).real() + 0.5);
                    p++;
                    res += uint64_t((*p).real() + 0.5) << 16;
                    p++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                }
                assert(carry == 0);
                capacity_ = uint32_t(n * sizeof(complex) / sizeof(uint32_t));
                ans = reinterpret_cast<uint32_t*>(p1);
            }
            else {
                operator delete(p1);
                ans = static_cast<uint32_t*>(operator new((m + k) * sizeof(uint32_t)));
                complex* p = p2;
                uint64_t carry = 0;
                for (uint32_t* ans_p = ans, *ans_p_end = ans_p + (m + k);
                    ans_p != ans_p_end; ++ans_p) {
                    uint64_t res = carry;
                    res += uint64_t((*p).real() + 0.5);
                    p++;
                    res += uint64_t((*p).real() + 0.5) << 8;
                    p++;
                    res += uint64_t((*p).real() + 0.5) << 16;
                    p++;
                    res += uint64_t((*p).real() + 0.5) << 24;
                    p++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                }
                assert(carry == 0);
                capacity_ = uint32_t(m + k);
            }
        }

        operator delete(nums_);
        nums_ = ans;
        int32_t sz = int32_t(m + k);
        int32_t sign_product = size_ ^ other.size_;
        size_ = sign_product >= 0 ? sz : -sz; 

        PopZeros();
        return *this;
    }

    inline LongInt operator*(const LongInt& other) const {
        LongInt copy(*this);
        copy *= other;
        return copy;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator==(int32_t n) const noexcept {
        bool not_same_sign = (size_ >= 0) == (n < 0);
        if (not_same_sign) {
            return false;
        }

        switch (size_) {
            case 0:
                return n == 0;
            case 1:
            case -1:
                return nums_[0] == uint32_t(std::abs(n));
            default:
                return false;
        }
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator==(int64_t n) const noexcept {
        bool not_same_sign = (size_ >= 0) == (n < 0);
        if (not_same_sign) {
            return false;
        }

        switch (size_) {
            case 0:
                return n == 0;
            case 1:
            case -1:
                return nums_[0] == uint64_t(std::abs(n));
            case 2:
            case -2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == uint64_t(std::abs(n));
            default:
                return false;
        }
    }

    inline constexpr bool operator==(uint32_t n) const noexcept {
        if (size_ < 0) {
            return false;
        }

        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            default:
                return false;
        }
    }

    inline constexpr bool operator==(uint64_t n) const noexcept {
        if (size_ < 0) {
            return false;
        }

        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == n;
            default:
                return false;
        }
    }

    inline constexpr bool operator!=(uint64_t n) const noexcept {
        return !(*this == n);
    }

#if _INTEGERS_128_BIT_
    constexpr bool operator==(uint128_t n) const noexcept {
        if (size_ < 0) {
            return false;
        }

        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == n;
            case 3: {
                uint64_t low = (uint64_t(nums_[1]) << 32) | nums_[0];
                return ((uint128_t(nums_[2]) << 64) | low) == n;
            }
            case 4: {
                uint64_t low = (uint64_t(nums_[1]) << 32) | nums_[0];
                uint64_t hi = (uint64_t(nums_[3]) << 32) | nums_[2];
                return ((uint128_t(hi) << 64) | low) == n;
            }
            default:
                return false;
        }
    }

    inline constexpr bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
#endif

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator==(const LongInt& other) const noexcept {
        return size_ == other.size_
            && memcmp(nums_, other.nums_, USize() * sizeof(uint32_t)) == 0;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator!=(const LongInt& other) const noexcept {
        return !(*this == other);
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator<(const LongInt& other) const noexcept {
        int32_t sign = GetSign();
        int32_t other_sign = other.GetSign();
        if (sign != other_sign) {
            return sign < other_sign;
        }

        if (size_ != other.size_) {
            return size_ < other.size_;
        }

        const uint32_t* nums = nums_ + size_ - 1;
        const uint32_t* other_nums = other.nums_ + size_ - 1;
        for (const uint32_t* end = nums_ - 1; nums != end; nums--, other_nums--) {
            if (*nums != *other_nums) {
                return ssize_t(*nums) * sign < ssize_t(*other_nums) * other_sign;
            }
        }

        return false;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator>(const LongInt& other) const noexcept {
        return other < *this;
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator<=(const LongInt& other) const noexcept {
        return !(other < *this);
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline bool operator>=(const LongInt& other) const noexcept {
        return !(*this <= other);
    }

    LongInt& operator+=(uint32_t x) {
        if (size_ == 0) {
            if (capacity_ == 0) {
                nums_ = static_cast<uint32_t*>(operator new(2 * sizeof(uint32_t)));
                capacity_ = 2;
            }

            nums_[0] = x;
            size_ = x != 0;
            return *this;
        }

        assert(capacity_ != 0);
        assert(size_ > 0 && "Not implemented for negative");
        uint32_t* it = nums_;
        uint32_t usize = uint32_t(std::abs(size_));
        const uint32_t* end = nums_ + usize;
        uint64_t carry = x;
        do {
            uint64_t res = uint64_t(*it) + carry;
            carry = res >> 32;
            *it = uint32_t(res);
            if (carry == 0) {
                goto add_u_end;
            }
            ++it;
        } while (it != end);

        if (carry != 0) {
            if (unlikely(usize == capacity_)) {
                GrowNotZeroCapacity();
            }

            assert(usize < capacity_);
            nums_[usize] = uint32_t(carry);
            size_++;
        }

    add_u_end:
        // size_ sign upd
        return *this;
    }

    LongInt& operator*=(uint32_t x) {   
        if (x == 0) {
            size_ = 0;
            return *this;
        }

        uint64_t carry = 0;
        uint64_t carry1 = 0;
        uint64_t b_0 = x;
        uint32_t usize = uint32_t(std::abs(size_));
        for (uint32_t* nums_it = nums_, * numes_it_end = nums_it + usize;
            nums_it != numes_it_end; ++nums_it) {
            uint64_t res = *nums_it * b_0 + carry;
            carry = res >> 32;
            uint64_t bit_sum = uint64_t(uint32_t(res)) + carry1;
            carry1 = bit_sum >> 32;
            *nums_it = uint32_t(bit_sum);
        }

        // nums_.emplace_back(carry + carry1);
        // PopZeros(); // sign will be updated here

        // carry1 == 0
        // x != 0 => sign wont change and there will be no leading zeros
        if (carry != 0) {
            if (unlikely(usize == capacity_)) {
                GrowCapacity();
            }

            assert(usize < capacity_);
            nums_[usize] = uint32_t(carry);
            size_ += GetSign();
        }

        return *this;
    }

    constexpr uint32_t DivideGetMod(uint32_t n) noexcept {
        uint64_t carry = 0;
        uint32_t* nums_iter_end = nums_ - 1;
        uint32_t* nums_iter = nums_iter_end + uint32_t(size_ >= 0 ? size_ : -size_);
        for (; nums_iter != nums_iter_end; --nums_iter) {
            uint64_t cur = (carry << 32) | uint64_t(*nums_iter);
            uint64_t q = cur / n;
            uint64_t r = cur % n;
            *nums_iter = uint32_t(q);
            carry = r;
        }

        PopZeros();
        return uint32_t(carry);
    }

    constexpr LongInt& operator>>=(uint32_t shift) noexcept {
        size_t size = USize();
        uint32_t uints_move = shift >> 5;
        if (uints_move >= size) {
            size_ = 0;
            return *this;
        }

        if (uints_move != 0) {
            memmove(nums_, nums_ + uints_move, (size - uints_move) * sizeof(uint32_t));
            size -= uints_move;
        }

        shift &= 0b11111;
        uint32_t* nums_iter = nums_;
        uint32_t* nums_iter_end = nums_iter + ssize_t(size) - 1;
        for (; nums_iter != nums_iter_end; ++nums_iter) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            *nums_iter = uint32_t(*reinterpret_cast<uint64_t*>(nums_iter) >> shift);
#else
            uint32_t hi = *(nums_iter + 1);
            uint32_t low = *nums_iter;
            uint32_t mask = (1 << shift) - 1;
            *nums_iter = ((hi & mask) << (32 - shift)) | (low >> shift);
#endif
        }

        *nums_iter_end >>= shift;
        size_ = size_ >= 0 ? int32_t(size) : -int32_t(size);
        return *this;
    }

    constexpr operator bool() noexcept {
        return size_ != 0;
    }

    constexpr void PopZeros() noexcept {
        size_t usize = USize();
        while (usize != 0 && nums_[usize - 1] == 0) {
            usize--;
        }

        size_ = size_ >= 0 ? int32_t(usize) : -int32_t(usize);
    }

    constexpr bool Empty() const noexcept {
        return size_ == 0;
    }

    constexpr int32_t Size() const noexcept {
        return size_;
    }

    constexpr size_t USize() const noexcept {
        /* cast to uint32_t to force zero extension when casting to size_t */
        return size_t(uint32_t(std::abs(size_)));
    }

    constexpr int32_t GetSign() const noexcept {
        return int32_t(size_ > 0) - int32_t(size_ < 0);
    }

    constexpr void ChangeSign() noexcept {
        size_ = -size_;
    }

    void FromString(std::string_view s) {
        const char* str_iter = s.begin();
        const char* str_end = s.end();
        int32_t sign = 1;
        while (str_iter != str_end && !std::is_digit(*str_iter)) {
            sign = 1 - (int32_t(*str_iter == '-') << 1);
            ++str_iter;
        }

        while (str_iter != str_end && *str_iter == '0') {
            ++str_iter;
        }

        if (str_iter == str_end) {
            size_ = 0;
            return;
        }

        uint32_t new_capacity = uint32_t(str_end - str_iter + 8) / 9;
        if (new_capacity > capacity_) {
            operator delete(nums_);
            nums_ = static_cast<uint32_t*>(operator new(new_capacity * sizeof(uint32_t)));
            capacity_ = new_capacity;
        }

        size_t usize = 1;
        if (size_t offset = size_t(str_end - str_iter) % 9) {
            uint32_t current = 0;
            while (offset--) {
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
            }

            nums_[0] = current;
        }
        else {
            assert(str_end - str_iter >= 9);
            uint32_t carry = uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            nums_[0] = carry;
        }

        assert(size_t(str_end - str_iter) % 9 == 0);
        while (str_iter != str_end) {
            uint64_t carry = 0;
            uint64_t carry1 = 0;
            for (uint32_t* nums_iter = nums_, * nums_iter_end = nums_ + usize; nums_iter != nums_iter_end; ++nums_iter) {
                uint64_t res = *nums_iter * 1'000'000'000ull + carry;
                carry = res >> 32;
                uint64_t bit_sum = uint64_t(uint32_t(res)) + carry1;
                carry1 = bit_sum >> 32;
                *nums_iter = uint32_t(bit_sum);
            }

            // carry1 == 0
            if (carry != 0) {
                if (unlikely(usize == capacity_)) {
                    GrowNotZeroCapacity();
                }

                assert(usize < capacity_);
                nums_[usize] = uint32_t(carry);
                usize++;
            }

            carry = uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;
            carry = carry * 10 + uint32_t(*str_iter) - '0';
            str_iter++;

            uint32_t* it = nums_;
            const uint32_t* end = nums_ + usize;
            do {
                uint64_t res = uint64_t(*it) + carry;
                carry = res >> 32;
                *it = uint32_t(res);
                if (carry == 0) {
#if __cplusplus >= 202302L
                    goto zero_carry:
#else
                    break;
#endif
                }
                ++it;
            } while (it != end);

            if (carry != 0) {
                if (unlikely(usize == capacity_)) {
                    GrowNotZeroCapacity();
                }

                assert(usize < capacity_);
                nums_[usize] = uint32_t(carry);
                usize++;
            }
#if __cplusplus >= 202302L
            zero_carry:
#endif
        }

        size_ = sign >= 0 ? int32_t(usize) : -int32_t(usize);
    }

    inline std::string ToString() const {
        std::string s;
        ToString(s);
        return s;
    }

    void ToString(std::string& ans) const {
        constexpr uint32_t digits_per_word = std::base_10_digits(uint64_t(uint32_t(-1)));
        size_t usize = USize();
        size_t max_number_len = (size_ <= 0) + digits_per_word * usize;
        char* digits = static_cast<char*>(operator new(max_number_len + 1));
        digits[max_number_len] = '\0';
        char* ptr = digits + max_number_len;
        size_t length = 0;
        if (size_ < 0) { *--ptr = '-'; length++; }

        uint32_t* nums_copy = static_cast<uint32_t*>(operator new(usize * sizeof(uint32_t)));
        memcpy(nums_copy, nums_, usize * sizeof(uint32_t));
        do {
            uint64_t carry = 0;
            for (uint32_t* iter_end = nums_copy - 1, * iter = iter_end + usize; iter != iter_end; --iter) {
                uint64_t cur = (carry << 32) | uint64_t(*iter);
                uint64_t q = cur / 10; // let compiler optimize it for us
                uint64_t r = cur % 10;
                *iter = uint32_t(q);
                carry = r;
            }

            while (usize != 0 && nums_copy[usize - 1] == 0) {
                usize--;
            }

            *--ptr = char('0' + carry);
            length++;
        } while (usize != 0);
        operator delete(nums_copy);

        ans = std::string(ptr, length);
        operator delete(digits);
    }

    inline friend std::ostream& operator<<(std::ostream& out, const LongInt& n) {
        if (n.size_ < 0) { out << '-'; }
        switch (n.size_) {
            case 0: {
                out << '0';
                break;
            }
            case 1:
            case -1: {
                out << n.nums_[0];
                break;
            }
            case 2:
            case -2: {
                out << ((uint64_t(n.nums_[1]) << 32) | n.nums_[0]);
                break;
            }
            default: {
                constexpr uint32_t digits_per_word = std::base_10_digits(uint64_t(uint32_t(-1)));
                size_t usize = n.USize();
                size_t max_number_len = digits_per_word * usize;
                char* digits = static_cast<char*>(operator new(max_number_len + 1));
                digits[max_number_len] = '\0';
                char* ptr = digits + max_number_len;
                size_t length = 0;
                uint32_t* nums_copy = static_cast<uint32_t*>(operator new(usize * sizeof(uint32_t)));
                memcpy(nums_copy, n.nums_, usize * sizeof(uint32_t));
                do {
                    uint64_t carry = 0;
                    for (uint32_t* iter_end = nums_copy - 1, * iter = iter_end + usize; iter != iter_end; --iter) {
                        uint64_t cur = (carry << 32) | uint64_t(*iter);
                        uint64_t q = cur / 10;
                        uint64_t r = cur % 10;
                        *iter = uint32_t(q);
                        carry = r;
                    }

                    while (usize != 0 && nums_copy[usize - 1] == 0) {
                        usize--;
                    }

                    *--ptr = char('0' + carry);
                    length++;
                } while (usize != 0);
                operator delete(nums_copy);
#if __GNUC__ && !defined(__clang__)
                __ostream_insert(out, ptr, length);
#else
                out << std::string_view(ptr, length);
#endif
                operator delete(digits);
            }
            break;
        }

        return out;
    }

    inline friend std::istream& operator>>(std::istream& in, LongInt& n) {
        std::string s;
        in >> s;
        n.FromString(s);
        return in;
    }

    inline void Reserve(uint32_t capacity) {
        if (capacity > capacity_) {
            uint32_t* new_nums = static_cast<uint32_t*>(operator new(capacity * sizeof(uint32_t)));
            size_t bsz = USize() * sizeof(uint32_t);
            if (bsz != 0) {
                memcpy(new_nums, nums_, bsz);
            }
            operator delete(nums_);
            nums_ = new_nums;
            capacity_ = capacity;
        }
    }

#if __cplusplus >= 202302L
    constexpr
#endif
    inline ~LongInt() {
        operator delete(nums_);
        nums_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }
protected:
    __attribute__((__noinline__)) void GrowNotZeroCapacity() {
        assert(capacity_ != 0);
        uint32_t new_capacity = capacity_ * 2;
        uint32_t* new_nums = static_cast<uint32_t*>(operator new(new_capacity * sizeof(uint32_t)));
        size_t sz = USize() * sizeof(uint32_t);
        if (sz != 0) {
            memcpy(new_nums, nums_, sz);
        }

        operator delete(nums_);
        nums_ = new_nums;
        capacity_ = new_capacity;
    }

    __attribute__((__noinline__)) void GrowCapacity() {
        // uint32_t new_capacity = capacity_ * 2;
        // new_capacity += (new_capacity == 0) << 1;
        uint32_t new_capacity = std::max(capacity_ * 2, 2u);
        uint32_t* new_nums = static_cast<uint32_t*>(operator new(new_capacity * sizeof(uint32_t)));
        size_t sz = USize() * sizeof(uint32_t);
        if (sz != 0) {
            memcpy(new_nums, nums_, sz);
        }

        operator delete(nums_);
        nums_ = new_nums;
        capacity_ = new_capacity;
    }
};

namespace std {
#if __cplusplus >= 202302L
    constexpr
#endif
    inline void swap(LongInt& n, LongInt& m) noexcept {
        n.Swap(m);
    }

    inline string to_string(const LongInt& n) {
        return n.ToString();
    }
}

#include <chrono>

void TestOperatorEqualsInt() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;

    constexpr int32_t K = 131072;
    for (int32_t i = -K; i < 0; i++) {
        n = i;
        assert(n.GetSign() == -1);
        assert(n.size_ == -1 && n.nums_[0] == uint32_t(-i));
    }
    n = 0;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (int32_t i = 1; i <= K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == uint32_t(i));
    }

    n = 0u;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (uint32_t i = 1; i < 2 * K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == i);
    }

    for (int64_t i = -K; i < 0; i++) {
        n = i;
        assert(n.GetSign() == -1);
        assert(n.size_ == -1 && n.nums_[0] == uint32_t(-i));
    }
    n = 0ll;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (int64_t i = 1; i <= K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == uint32_t(i));
    }

    n = 0ull;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (uint64_t i = 1; i < 2 * K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == i);
    }

    n = uint128_t(0);
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    n = uint128_t(-1);
    assert(n.GetSign() == 1);
    assert(n.size_ == 4);
    assert(n.nums_[0] == uint32_t(-1)
        && n.nums_[1] == uint32_t(-1)
        && n.nums_[2] == uint32_t(-1)
        && n.nums_[3] == uint32_t(-1));
}

void TestLongIntMult() {
    puts(__PRETTY_FUNCTION__);
    LongInt n1;
    LongInt n2;
    constexpr uint64_t K = 6000;
    for (uint64_t i = 1; i <= K; i++) {
        for (uint64_t j = 1; j <= K; j++) {
            n1 = i;
            n2 = j;
            assert((n1 < n2) == (i < j));
            n1 *= n2;
            assert(n1 == i * j);
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint64_t j = uint64_t(-1) - K; j != 0; j++) {
            n1 = uint64_t(i);
            n2 = uint64_t(j);
            assert((n1 < n2) == (i < j));
            n1 *= n2;
            assert(n1 == (uint128_t(i) * uint128_t(j)));
        }
    }

    const char* test_cases[][3] = {
        { "0", "0", "0" },
        { "0", "1", "0" },
        { "1", "0", "0" },
        { "0", "99999999999999999999999999999999999999999999999999999999999", "0" },
        { "99999999999999999999999999999999999999999999999999999999999", "0", "0" },
        { "1", "1", "1" },
        { "2", "1", "2" },
        { "1", "2", "2" },
        { "10", "20", "200" },
        { "12", "20", "240" },
        { "1024", "4", "4096" },
        { "12000000", "20000000", "240000000000000" },
        { "13721838", "317826897", "4361169192676686" },
        { "317826897", "13721838", "4361169192676686" },
        { "131241234423234", "5984348957348", "785393344381744834046223432" },
        { "340282367000166625996085689103316680705",
            "340282367000166625996085689103316680705",
            "115792089291236088776703569810027986869841637790274196431140198001898919297025" },
        { "2734678246872364872368472638742364827364287648273648132318892173901848293489172492348923789614023527938467235423498238492794323784728935349239875",
            "2348902185132056348724237831450938453094157098145039751309847593814914534715981347589134057893417",
            "6423491709711598889041115876654018649432963188732726742064025989984811440018290126586200884641329217870700376754042411607663158208477573260850076156608872687420427040889726888717004048685350284582553498126187128453709828981195657834716402875" },
        { "952263744", "9359517973", "8912729627004270912" },
        { "4219788824", "2743656178", "11577649676822954672" },
        { "2968434375", "517784556", "1537009474874512500" },
        { "84505427146800725206214423388331447259",
            "279405665168189620898465228388855751530",
            "23611295082273845004407889189114051194515549345238521714122644008631003556270" },
        { "241624483789163450827236083147698994361",
            "176969948248112551859871363564234198810",
            "42760272391645166121149990905372750336999611475675972607486869021321542910410" },
        { "189745687871346578913675631984859831398756913846314756",
            "23423242342342342342342342345689457967408570698981759840534",
            "4444459230424999019002887192350710359853726020388674142289774769673240565480827228712761450490207639434131119704" },
        { "23423242342342342342342342345689457967408570698981759840534",
            "189745687871346578913675631984859831398756913846314756",
            "4444459230424999019002887192350710359853726020388674142289774769673240565480827228712761450490207639434131119704" },
        { "4979013912031336395843652482056944541917879830658888080210860546945162316784007696722472104490292115006929224505561952621549644507506157905895141883351693357671866673372387489064601790435943279753076135508734976402986590141791640275729075110301052182525982122055945308822786348774541301190917267056010293522021711953908722426988170600486754359151355279642107326020710320811965589623077577919604276793995290693241605498933690131016500499830411351885329358828099445375214953095242883288551175600040639508973290618254632656708077698032644332404531654194661240327089976520228558815070547050483607370290128880665115493428583195942214144370252680416623185742126651651971205877033380835086928683006777647705797323226637754670943913955499629956581438051579033788646418455294222226794208542773240250813577595297820571439025260443186083647739824886601706060545130979337849454640894799067017660651029798759710161866959877729945261418881826853299275537913940529803748601533888556248642308523858954129309895839621",
            "1089706208601124071370814153227426268538693661536077492693402002113706077836908897064010719715176055629119821718280985463346231823617831015276226748067975032405343094828082657769205252259131474831109973291257749293454119745786044094162983334244656183437048486586190591260478069683126170080375767206116766997111495034910226069855175658687975019210684025876539532495651085887043273971291745246957837630873649994471012335459828190836962052822964761370237541987899251435611120642920093504465781338431847062674488228328048386887619645965298994056431751145798374214112096174986469396938700846690701869472565650854341011865979075747215621155738285771206033348621609414969877385311942166051820149735596291338889291251880106483211920648684916159659287049528587849488130282624723754842322308381390280175920445345519401578177975718271424021934524065020159121707342876220556416710152269947564613985386481137040800062435273812000700729504595716591612580528925322234310669034643636549786097960658822655773576934232",
            "5425662372651918275299375148629540006461321920756701170286725562568759229810880572911769849537355451610005569960228002391215085052847571622689094832054985279145552749161681046861526713162440534136296490785928442215744545711439342983442729982749120390733490907009647021054311170524633132570266240806829026546139687602916900229654645538055150485456658149197560778991707820278152313692132979576603692190567330432232166508406117583864266519222289887768101694869359057868279715246089989344254809355928718476453294822032928508718003882820121545975530953496070612041244576392616141309842246017940303977167606784428044629301092398352005265891045586737344102752622286046610267838174215047313315660881597083337684595892961583985896458747267807898419659791494353138717845748556312008417645821599206848619748895452043304609685863025172031227579738617956645158235248258575509480721560219448272650065950696046407386099543681594741302423741017536949076337719309499789273159526229632009103995883101494232640178582443664188171605651617679364610071764342694526545790368273219400375205037157668346710049634975697352390763663222195249887562718049391850108222280866744148593795622184880213367433062797045761596438730675980702578848090758612689638246878037965734766568789946008561922300075077798552095915029193968953456136549672456085235618704291776967924718538238249798636597249940675408244486413603110202278103459733616978591497974412117657019479952731294916161646997813925500899342321384352570569765496029832893163507647823951079695506656446830692581049404739304276990177534134151038266962485292415265257738018558654770901977444563735949266542891330128734419196999200268758519164113373322554156512358342313975777464929251035097242190670537217696832588167811748620251129869183988486287430476024319388751623800595497573205109796023980368739722164681252677155919289466458420249924228564533145913434727157758360817904213671207456616337309903686438079934167085071256457656974379118052493980516605329326027531228264736806072" },
        { "1089706208601124071370814153227426268538693661536077492693402002113706077836908897064010719715176055629119821718280985463346231823617831015276226748067975032405343094828082657769205252259131474831109973291257749293454119745786044094162983334244656183437048486586190591260478069683126170080375767206116766997111495034910226069855175658687975019210684025876539532495651085887043273971291745246957837630873649994471012335459828190836962052822964761370237541987899251435611120642920093504465781338431847062674488228328048386887619645965298994056431751145798374214112096174986469396938700846690701869472565650854341011865979075747215621155738285771206033348621609414969877385311942166051820149735596291338889291251880106483211920648684916159659287049528587849488130282624723754842322308381390280175920445345519401578177975718271424021934524065020159121707342876220556416710152269947564613985386481137040800062435273812000700729504595716591612580528925322234310669034643636549786097960658822655773576934232",
            "4979013912031336395843652482056944541917879830658888080210860546945162316784007696722472104490292115006929224505561952621549644507506157905895141883351693357671866673372387489064601790435943279753076135508734976402986590141791640275729075110301052182525982122055945308822786348774541301190917267056010293522021711953908722426988170600486754359151355279642107326020710320811965589623077577919604276793995290693241605498933690131016500499830411351885329358828099445375214953095242883288551175600040639508973290618254632656708077698032644332404531654194661240327089976520228558815070547050483607370290128880665115493428583195942214144370252680416623185742126651651971205877033380835086928683006777647705797323226637754670943913955499629956581438051579033788646418455294222226794208542773240250813577595297820571439025260443186083647739824886601706060545130979337849454640894799067017660651029798759710161866959877729945261418881826853299275537913940529803748601533888556248642308523858954129309895839621",
            "5425662372651918275299375148629540006461321920756701170286725562568759229810880572911769849537355451610005569960228002391215085052847571622689094832054985279145552749161681046861526713162440534136296490785928442215744545711439342983442729982749120390733490907009647021054311170524633132570266240806829026546139687602916900229654645538055150485456658149197560778991707820278152313692132979576603692190567330432232166508406117583864266519222289887768101694869359057868279715246089989344254809355928718476453294822032928508718003882820121545975530953496070612041244576392616141309842246017940303977167606784428044629301092398352005265891045586737344102752622286046610267838174215047313315660881597083337684595892961583985896458747267807898419659791494353138717845748556312008417645821599206848619748895452043304609685863025172031227579738617956645158235248258575509480721560219448272650065950696046407386099543681594741302423741017536949076337719309499789273159526229632009103995883101494232640178582443664188171605651617679364610071764342694526545790368273219400375205037157668346710049634975697352390763663222195249887562718049391850108222280866744148593795622184880213367433062797045761596438730675980702578848090758612689638246878037965734766568789946008561922300075077798552095915029193968953456136549672456085235618704291776967924718538238249798636597249940675408244486413603110202278103459733616978591497974412117657019479952731294916161646997813925500899342321384352570569765496029832893163507647823951079695506656446830692581049404739304276990177534134151038266962485292415265257738018558654770901977444563735949266542891330128734419196999200268758519164113373322554156512358342313975777464929251035097242190670537217696832588167811748620251129869183988486287430476024319388751623800595497573205109796023980368739722164681252677155919289466458420249924228564533145913434727157758360817904213671207456616337309903686438079934167085071256457656974379118052493980516605329326027531228264736806072" },
        { "39874589736285746348537648975364987536485463475689312758346175316947648753619457374157823617426913817847816718871267134057345147516847576813468751364056130456875613458136745",
            "348",
            "13876357228227439729291101843427015662696941289539880839904469010297781766259571166206922618864566008611040218167200962651956111335862956731087125474691533398992713483431587260" },
        { "348",
            "39874589736285746348537648975364987536485463475689312758346175316947648753619457374157823617426913817847816718871267134057345147516847576813468751364056130456875613458136745",
            "13876357228227439729291101843427015662696941289539880839904469010297781766259571166206922618864566008611040218167200962651956111335862956731087125474691533398992713483431587260" },
        { "378473591711125662205318978307400782427783753923984686802121246042271316236774444461081305720113769234627500055999892631802251943506510613041576973905140659553198302268762932232635013075121844024010844209617001387445367992616947475847556277530823124716511381308443893601036689835540126414069661052087138994082036710162383386369932806029019860996789535859361485418449897433469146302768382820057178659536255110169359845196380565725061526581227067549629866724246482931073170051376781704197212768365253140559066042002101297055200613269143572019112557085563899878475510810554198342040963733144408262697470764570314944593526613934419263025337497357042571717671759023883586867451377168248733405883910296788303702062215452776471512256941837484725160214833729957202282648704994064553439348115504885342193852929906418451561210887575546337507615915632010857173564415901485308965735447198738488397867151647504526550040119974630239332996345427278593883623686974957277376278631354869639366838460468748101319039408427701123993442888448154403121426916704627733981704445112370876108453961872801955347846801218254087739489562170030806198132295724995482071830436648020515150300468035898691623269527628850137584131992179304108104221244082970339981314645",
            "242422360745395402404187893934203477400974449065647049988927198970319805753149705988973274875934705741319528222056518230978113788601185987597650863481917975147889480918426062465619374716968961931781307592900017970779362645595784950197534818861618814548119308868190069641017565582001182178863908348366042579709934563168161190371007834351658980901439537387725151710448936716057525234422021079168107409249681886185803656798184787755755302137461956528885902748067318538923549260818296551459793281096136811002727591059360446183594804912759682504651810130132241402424826964162913576719514934406842887250631366029807679608779995452685983217009380013356826166862603999974178446346927185519163124450042997576507464420018043214571816883931340327689828911413087854407416013902322113202969761256247555360669534677298033285373411229119624362114218631744679287995208711756999916330336680306667802886976862157913540009816489722867943929152313666052647540305002671654304297771170195703695712037696137032492879795700752067767022685233264574030743197818202098648173117308846148560570005975871944017411174794281674815109904233256841885601132453369966630233504941985437823039274173244307950067942092770927309358035074043249749436812689885822328999166497",
            "91750461582399996481249056069552341061631977362166795955654445790264222859720389211624815079283843625975863399889074212216266297134990815564761330594156985225047530954070381961437148213195600057969015010386733547822475958080115840136741686692338859325504867646221723989400550500929317397416221571891625767783321077342877108720012126080679211874940105656574765376297890689604436113946845444191558673952067190703736606117892318167589626611808999592345128060713013723521842461568316927463862910923034086153951336451168076535477516830914439285093043762990954039413381588521712360446061593810377394657345416121564517043424860788547525235085792343018997727948065012605965408721320163051985218148442070481328417606499793006079912309477825377935641101410477501161932089027628535008552104222618408935767393335060030694945758378704672264522319383457997330369049702343188167604623366678376120989861617610630672930536465566980876948021796581916334213547308250522630241086319046806456977746512235528072488535874371368947268698209015357823085409795719588097028234042794879605958978464183072895599180811154590444271971396865304247122008224327128637067378970116215511700712030353712423319327512511897289586567993904360468930853505705469168815995897919297049268364364202469557536979483426732147221671109127696683092476140491126818755624998935111987394755116214733724671355006980468468795377673992716072110654551008812212873841685531210505119906739350111475048911270730349061872269078463062821935188053118917685774942151561764762561390599264509475929928149502026146850838330252467296169912031042785085004360846295411799159444104152561578205952489721730980991738242222877578374533496541619714851354580401742391750350373448393935628587788954777352482566911435843918432923932547136028133093518824417001358921720176306869776325301163185814759856575836089499945845246719250766361998726161201416467077119735405112368483599218727089596693298272073242067820420384714345537586697308993657001146038822806716503143415537504249771740386887313648047613127969728987677190328129661590559820669047704931339203348331237658184130719236353882236820168212391548475189594072837236345386281884700821238193120175068335949608964486690094889051463543949903889705367037538550238926930991984064029903921425133938921567363993240377805736468371128960994609947805802949073455360925189610758755620972319942874745920155808369257461687434973249064848189008369934903417392669574477053593427918093472288422759299448565" },
    };
    for (const char *const (&test)[3] : test_cases) {
        n1.FromString(test[0]);
        n2.FromString(test[1]);
        n1 *= n2;
        n2.FromString(test[2]);
        if (n1 != n2) {
            cout << test[2] << ' ' << strlen(test[2]) << '\n';
            cout << n1 << ' ' << n1.Size() << '\n';
            return;
        }
    }

    namespace chrono = std::chrono;

    constexpr size_t k = size_t(1e6);
    string s1(k, '9');
    n1.FromString(s1);
    {
        auto start = chrono::high_resolution_clock::now();
        n1 *= n1;
        auto end = chrono::high_resolution_clock::now();
        printf("Multiplied 1000000 digit numbers in %llu ms\n",
            uint64_t(chrono::duration_cast<chrono::milliseconds>(end - start).count()));
    }
    string ans(2 * k, '\0');
    memset(ans.data(), '9', (k - 1) * sizeof(char));
    ans[k - 1] = '8';
    memset(ans.data() + k, '0', (k - 1) * sizeof(char));
    ans[2 * k - 1] = '1';
    n2.FromString(ans);
    if (n1 != n2) { cout << "Long test failed\n"; }
}

void TestUIntMult() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    constexpr uint64_t K = 6000;
    for (uint64_t i = 0; i <= K; i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            n *= j;
            assert(n == i * j);
        }
    }

    for (uint128_t i = (uint128_t(-1) / K) - K; i != (uint128_t(-1) / K); i++) {
        for (uint32_t j = 0; j < K; j++) {
            n = i;
            n *= j;
            assert(n == i * j);
        }
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n *= uint32_t(j);
            assert(n == uint64_t(i) * uint64_t(j));
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n *= j;
            assert(n == uint128_t(i) * j);
        }
    }
}

void TestUIntAdd() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    constexpr uint64_t K = 6000;
    for (uint64_t i = 0; i <= K; i++) {
        for (uint64_t j = 0; j <= K; j++) {
            n = i;
            assert(n == i);
            n += uint32_t(j);
            assert(n == i + j);
        }
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == uint64_t(i));
            n += j;
            assert(n == uint64_t(i) + uint64_t(j));
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == i);
            n += j;
            assert(n == uint128_t(i) + uint128_t(j));
        }
    }

    for (uint128_t i = uint64_t(-1); i != uint128_t(uint64_t(-1)) + 2 * K; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == i);
            n += j;
            assert(n == i + j);
        }
    }
}

void TestFromString() {
    puts(__PRETTY_FUNCTION__);
    constexpr int64_t numbersI64[] = {
        0,
        1,
        -1,
        2,
        -2,
        4,
        -4,
        8,
        -8,
        9,
        -9,
        10,
        -10,
        11,
        -11,
        2147483648,
        -2147483648,
        4294967295,
        -4294967295,
        4294967296,
        -4294967296,
        131241234423234ll,
        5984348957348ll,
        9223372036854775807ll,
        -9223372036854775807ll,
        -9223372036854775807ll - 1,
    };

    LongInt n;
    for (int64_t number : numbersI64) {
        n.FromString(std::to_string(number));
        assert(n == number);
    }

    constexpr uint64_t numbersU64[] = {
        0,
        1,
        2,
        4,
        8,
        9,
        10,
        11,
        2147483648,
        4294967295,
        4294967296,
        9223372036854775808ull,
        18446744073709551615ull,
    };
    for (uint64_t number : numbersU64) {
        n.FromString(std::to_string(number));
        assert(n == number);
    }

    constexpr uint128_t numbersU128[] {
        0,
        1,
        2,
        4,
        8,
        ((uint128_t(42576258ull) << 64) | uint128_t(9439515947379090504ull)),
        (uint128_t(4581048384968843434ull) << 64) | (uint128_t(15881123738085757915ull)),
        (uint128_t(15146611459005431080ull) << 64) | (uint128_t(11003818173265126250ull)),
        (uint128_t(107408329755340997ull) << 64) | (uint128_t(10118848797326968254ull)),
        (uint128_t(15406421307076602009ull) << 64) | (uint128_t(3266360438134194608ull)),
        (uint128_t(13098489512494978084ull) << 64) | (uint128_t(13194323124312210617ull)),
        (uint128_t(9593560117762545909ull) << 64) | (uint128_t(6883651453229059866ull)),
        (uint128_t(uint64_t(-1)) << 64) | (uint128_t(uint64_t(-1)))
    };
    for (uint128_t number : numbersU128) {
        n.FromString(std::to_string(number));
        assert(n == number);
    }

    uint128_t c = 0;
    std::string s;
    s.reserve(39);
    for (size_t i = 0; i < 39; i++) {
        n.FromString(s);
        assert(n == c);
        s.push_back('9');
        c = c * 10 + 9;
    }
}

void TestToString() {
    puts(__PRETTY_FUNCTION__);
    constexpr int64_t numbersI64[] = {
        0,
        1,
        -1,
        2,
        -2,
        4,
        -4,
        8,
        -8,
        9,
        -9,
        10,
        -10,
        11,
        -11,
        2147483648,
        -2147483648,
        4294967295,
        -4294967295,
        4294967296,
        -4294967296,
        131241234423234ll,
        5984348957348ll,
        9223372036854775807ll,
        -9223372036854775807ll,
        -9223372036854775807ll - 1,
    };

    LongInt n;
    for (int64_t number : numbersI64) {
        n.FromString(std::to_string(number));
        assert(n == number);
    }

    constexpr uint64_t numbersU64[] = {
        0,
        1,
        2,
        4,
        8,
        9,
        10,
        11,
        2147483648,
        4294967295,
        4294967296,
        9223372036854775808ull,
        18446744073709551615ull,
    };

    for (uint64_t number : numbersU64) {
        n = number;
        assert(n.ToString() == std::to_string(number));
    }

    constexpr uint128_t numbersU128[] {
        0,
        1,
        2,
        4,
        8,
        ((uint128_t(42576258ull) << 64) | uint128_t(9439515947379090504ull)),
        (uint128_t(4581048384968843434ull) << 64) | (uint128_t(15881123738085757915ull)),
        (uint128_t(15146611459005431080ull) << 64) | (uint128_t(11003818173265126250ull)),
        (uint128_t(107408329755340997ull) << 64) | (uint128_t(10118848797326968254ull)),
        (uint128_t(15406421307076602009ull) << 64) | (uint128_t(3266360438134194608ull)),
        (uint128_t(13098489512494978084ull) << 64) | (uint128_t(13194323124312210617ull)),
        (uint128_t(9593560117762545909ull) << 64) | (uint128_t(6883651453229059866ull)),
        (uint128_t(uint64_t(-1)) << 64) | (uint128_t(uint64_t(-1)))
    };
    for (uint128_t number : numbersU128) {
        n = number;
        assert(n.ToString() == std::to_string(number));
    }

    uint128_t c = 0;
    std::string s;
    std::string buffer;
    s.reserve(39);
    buffer.reserve(39);
    n = c;
    n.ToString(buffer);
    assert(buffer.size() == 1 && buffer[0] == '0');
    for (size_t i = 0; i < 38; i++) {
        s.push_back('9');
        c = c * 10 + 9;
        n = c;
        assert(n == c);
        n.ToString(buffer);
        assert(buffer == s);
    }
}

void TestBitShifts() {
    constexpr uint32_t k = 4096;
    LongInt n;
    n.Reserve(4);
    for (uint32_t i = 0; i <= k; i++) {
        for (uint32_t shift = 0; shift <= 31; shift++) {
            n = i;
            n >>= shift;
            assert(n == (i >> shift));
        }
        for (uint32_t j = 0; j <= 16; j++) {
            n = i;
            n >>= 32 + j;
            assert(n == 0);
        }
    }

    for (uint128_t i = uint128_t(-1) - k; i != 0; i++) {
        for (uint32_t shift = 0; shift <= 127; shift++) {
            n = i;
            n >>= shift;
            if (n != (i >> shift)) {
                cout << i << '\n' << n << '\n' << shift;
                return;
            }
            assert(n == (i >> shift));
        }
        for (uint32_t j = 0; j <= 16; j++) {
            n = i;
            n >>= 128 + j;
            assert(n == 0);
        }
    }

    // 1 << 255
    n.FromString("57896044618658097711785492504343953926634992332820282019728792003956564819968");
    LongInt m;
    m.Reserve(uint32_t(n.USize()));
    for (uint32_t shift = 0; shift <= 127; shift++) {
        m = n;
        m >>= (255 - shift);
        assert(m == uint128_t(1) << shift);
    }
}

int main() {
    // TestOperatorEqualsInt();
    // TestLongIntMult();
    // TestUIntMult();
    // TestUIntAdd();
    // TestFromString();
    // TestToString();
    // TestBitShifts();
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    LongInt n1;
    LongInt n2;
    cin >> n1 >> n2;
    n1 *= n2;
    cout << n1;
    cout.flush();
    return 0;
}

// g++ -std=c++2b -Wall -Wextra -Wpedantic -Werror -Wunused --pedantic-error -Wconversion -Wshadow -Wnull-dereference -Warith-conversion -Wcast-align -Warray-bounds=2 -Ofast -march=native .\LongInt.cpp -o LongInt.exe
