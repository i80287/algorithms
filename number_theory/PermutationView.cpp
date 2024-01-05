#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <numeric>

#if __cplusplus >= 202002L
#define PermutationViewConstexpr constexpr
#else
#define PermutationViewConstexpr
#endif

class PermutationView {
public:
    PermutationViewConstexpr PermutationView() noexcept(
        noexcept(std::vector<uint32_t>{})) = default;

    PermutationViewConstexpr explicit PermutationView(size_t length)
        : elems_(length) {
        uint32_t i = 0;
        for (uint32_t& elem : elems_) {
            elem = ++i;
        }
    }

    explicit PermutationView(const std::vector<uint32_t>& elems)
        : elems_{elems} {
        CheckElems(elems.begin(), elems.end());
    }

    explicit PermutationView(std::initializer_list<uint32_t> elems)
        : elems_{elems} {
        CheckElems(elems.begin(), elems.end());
    }

    PermutationViewConstexpr void resize(size_t new_size) {
        elems_.resize(new_size);
        uint32_t i = 0;
        for (uint32_t& elem : elems_) {
            elem = ++i;
        }
    }

    PermutationViewConstexpr size_t size() const noexcept {
        return elems_.size();
    }

    PermutationViewConstexpr bool empty() const noexcept {
        return elems_.empty();
    }

    PermutationViewConstexpr bool CheckNumber(size_t i) const noexcept {
        return i - 1 < size();
    }

    PermutationView& Swap(uint32_t i, uint32_t j) {
        if (!CheckNumber(i) || !CheckNumber(j)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }

        std::swap(elems_[i - 1], elems_[j - 1]);
        return *this;
    }

    PermutationViewConstexpr uint32_t& operator[](size_t number) {
        // see https://en.cppreference.com/w/cpp/language/constexpr
        return CheckNumber(number)
                   ? elems_[number - 1]
                   : throw std::out_of_range(__PRETTY_FUNCTION__);
    }

    PermutationViewConstexpr uint32_t operator[](size_t number) const {
        // see https://en.cppreference.com/w/cpp/language/constexpr
        return CheckNumber(number)
                   ? elems_[number - 1]
                   : throw std::out_of_range(__PRETTY_FUNCTION__);
    }

    std::string to_string() const {
        const size_t n = elems_.size();
        const size_t max_digits_count = std::to_string(n).size();
        const size_t one_line_length =
            n * max_digits_count + (n - 1);  // n - 1 spaces
        // 1 for '\n', 8 for "/ ", "\ ", " \", " /"
        const size_t capacity = one_line_length + 1 + one_line_length + 8;
        std::string s;
        s.reserve(capacity);
        s.push_back('/');
        s.push_back(' ');

        for (uint32_t i = 1; i <= n; i++) {
            std::string str_i_repr = std::to_string(i);
            size_t needed_spaces_count = max_digits_count - str_i_repr.size();
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
            size_t needed_spaces_count = max_digits_count - str_i_repr.size();
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

    friend std::ostream& operator<<(std::ostream& out,
                                    const PermutationView& perm) {
        return out << perm.to_string();
    }

protected:
    template <class Iterator>
    void CheckElems(Iterator begin, Iterator end) const noexcept(false) {
        for (; begin != end; ++begin) {
            if (!CheckNumber(*begin)) {
                throw std::out_of_range(__PRETTY_FUNCTION__);
            }
        }
    }

    std::vector<uint32_t> elems_;
};

namespace std {
inline string to_string(const PermutationView& permutation) {
    return permutation.to_string();
}
}  // namespace std

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
    p.Swap(1, 2);
    p.Swap(10, 9);
    std::cout << p << "\n\n";

    p.resize(50);
    p.Swap(1, 50);
    p.Swap(2, 49);
    std::cout << p;
}
