#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "../misc/config_macros.hpp"

#if CONFIG_VECTOR_SUPPORTS_CONSTEXPR_OPERATIONS
#define CONSTEXPR_VECTOR constexpr
#else
#define CONSTEXPR_VECTOR inline
#endif

class PermutationView final {
    using Container = std::vector<uint32_t>;

public:
    using size_type = typename Container::size_type;

    CONSTEXPR_VECTOR PermutationView() = default;

    CONSTEXPR_VECTOR explicit PermutationView(size_type length) : elems_(length) {
        std::iota(elems_.begin(), elems_.end(), 1);
    }

    explicit PermutationView(std::vector<uint32_t> elems) noexcept : elems_(std::move(elems)) {
        CheckElems(elems_.begin(), elems_.end());
    }

    explicit PermutationView(std::initializer_list<uint32_t> elems) : elems_(elems) {
        CheckElems(elems_.begin(), elems_.end());
    }

    CONSTEXPR_VECTOR void resize(size_type new_size) {
        elems_.resize(new_size);
        uint32_t i = 0;
        for (uint32_t& elem : elems_) {
            elem = ++i;
        }
    }

    CONSTEXPR_VECTOR size_type size() const noexcept {
        return elems_.size();
    }

    CONSTEXPR_VECTOR bool empty() const noexcept {
        return elems_.empty();
    }

    CONSTEXPR_VECTOR bool CheckNumber(size_type i) const noexcept {
        return i - 1 < size();
    }

    CONSTEXPR_VECTOR PermutationView& Swap(uint32_t i, uint32_t j) {
        if (!CheckNumber(i) || !CheckNumber(j)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }

        std::swap(elems_[i - 1], elems_[j - 1]);
        return *this;
    }

    CONSTEXPR_VECTOR uint32_t& operator[](size_type number) {
        // see https://en.cppreference.com/w/cpp/language/constexpr
        return CheckNumber(number) ? elems_[number - 1]
                                   : throw std::out_of_range(__PRETTY_FUNCTION__);
    }

    CONSTEXPR_VECTOR uint32_t operator[](size_type number) const {
        // see https://en.cppreference.com/w/cpp/language/constexpr
        return CheckNumber(number) ? elems_[number - 1]
                                   : throw std::out_of_range(__PRETTY_FUNCTION__);
    }

    std::string to_string() const {
        const size_type n = elems_.size();
        const size_type max_digits_count = std::to_string(n).size();
        const size_type one_line_length = n * max_digits_count + (n - 1);  // n - 1 spaces
        // 1 for '\n', 8 for "/ ", "\ ", " \", " /"
        const size_type capacity = one_line_length + 1 + one_line_length + 8;
        std::string s;
        s.reserve(capacity);
        s.push_back('/');
        s.push_back(' ');

        for (uint32_t i = 1; i <= n; i++) {
            std::string str_i_repr = std::to_string(i);
            size_type needed_spaces_count = max_digits_count - str_i_repr.size();
            // Unroll while loop for small i space padding
            switch (needed_spaces_count) {
                case 2:
                    s.push_back(' ');
                    [[fallthrough]];
                case 1:
                    s.push_back(' ');
                    [[fallthrough]];
                case 0:
                    break;
                default:
                    do {
                        s.push_back(' ');
                    } while (--needed_spaces_count != 0);
                    break;
            }

            s.append(str_i_repr);
            s.push_back(' ');
        }

        // ' ' before '\' is already pushed in the cycle
        s.append("\\\n\\ ");

        for (uint32_t i = 0; i < n; i++) {
            std::string str_i_repr = std::to_string(elems_[i]);
            size_type needed_spaces_count = max_digits_count - str_i_repr.size();
            // Unroll while loop for small i space padding
            switch (needed_spaces_count) {
                case 2:
                    s.push_back(' ');
                    [[fallthrough]];
                case 1:
                    s.push_back(' ');
                    [[fallthrough]];
                case 0:
                    break;
                default:
                    do {
                        s.push_back(' ');
                    } while (--needed_spaces_count != 0);
                    break;
            }

            s.append(str_i_repr);
            s.push_back(' ');
        }

        // ' ' before '/' is already pushed in the cycle
        s.push_back('/');

        return s;
    }

    friend std::ostream& operator<<(std::ostream& out, const PermutationView& perm) {
        return out << perm.to_string();
    }

    friend std::string to_string(const PermutationView& permutation) {
        return permutation.to_string();
    }

    CONSTEXPR_VECTOR friend void swap(PermutationView& lhs, PermutationView& rhs) noexcept {
        lhs.elems_.swap(rhs.elems_);
    }

protected:
    template <class Iterator>
    void CheckElems(Iterator begin, Iterator end) const {
        for (; begin != end; ++begin) {
            if (!CheckNumber(*begin)) {
                throw std::out_of_range(__PRETTY_FUNCTION__);
            }
        }
    }

    Container elems_;
};

int main() {
    /**
     * Prints out
     *
     * /  1  2  3  4  5  6  7  8  9 10 \
     * \  1  2  3  4  5  6  7  8  9 10 /
     *
     * /  1  2  3  4  5  6  7  8  9 10 \
     * \  2  1  3  4  5  6  7  8 10  9 /
     */
    PermutationView p(10);
    std::cout << p << "\n\n";
    assert(p.to_string() ==
           "/  1  2  3  4  5  6  7  8  9 10 \\\n"
           "\\  1  2  3  4  5  6  7  8  9 10 /");

    p.Swap(1, 2);
    p.Swap(10, 9);
    std::cout << p << "\n\n";
    assert(p.to_string() ==
           "/  1  2  3  4  5  6  7  8  9 10 \\\n"
           "\\  2  1  3  4  5  6  7  8 10  9 /");

    p.resize(50);
    p.Swap(1, 50);
    p.Swap(2, 49);
    std::cout << p << '\n';
    assert(p.to_string() ==
           "/  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 "
           "29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 \\\n"
           "\\ 50 49  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 "
           "29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48  2  1 /");
}
