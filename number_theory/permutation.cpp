#include <cassert>
#include <cstdint>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <string>
#include <iterator>
#include <iostream>
#include <initializer_list>
#include <vector>

class Permutation {
public:
    inline constexpr Permutation() noexcept(noexcept(std::vector<uint32_t>{})) = default;

    inline constexpr explicit Permutation(size_t length) : elems_(length) {
        uint32_t i = 0;
        for (uint32_t& elem : elems_) {
            elem = ++i;
        }

        assert(i == Size());
    }

    inline constexpr explicit Permutation(const std::vector<uint32_t>& elems) noexcept(false) : elems_{elems} {
        CheckElems(elems.begin(), elems.end());
    }

    inline constexpr explicit Permutation(std::initializer_list<uint32_t> elems) noexcept(false) : elems_{elems} {
        CheckElems(elems.begin(), elems.end());
    }

    inline constexpr size_t Size() const noexcept {
        return elems_.size();
    }

    inline constexpr bool CheckNumber(size_t i) const noexcept {
        return i - 1 < Size();
    }

    inline constexpr Permutation& Swap(uint32_t i, uint32_t j) noexcept(false) {
        if (!CheckNumber(i) || !CheckNumber(j)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }

        std::swap(elems_[i - 1], elems_[j - 1]);
        return *this;
    }

    inline constexpr uint32_t& operator[](size_t number) noexcept(false) {
        if (!CheckNumber(number)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }

        return elems_[number - 1];
    }

    inline constexpr uint32_t operator[](size_t number) const noexcept(false) {
        if (!CheckNumber(number)) {
            throw std::out_of_range(__PRETTY_FUNCTION__);
        }

        return elems_[number - 1];
    }

    std::string ToString() const {
        size_t n = elems_.size();
        if (n == 0) {
            return "/ \\\n\\ /";
        }
        
        size_t max_digits_count = std::to_string(n).size();
        size_t one_line_length = n * max_digits_count + (n - 1); // n - 1 spaces
        std::string s;
        s.reserve(one_line_length + 1 + one_line_length + 8); // 1 for '\n', 8 for "/ ", "\ ", " \", " /"
        s.push_back('/');
        s.push_back(' ');

        for (uint32_t i = 1; i <= n; i++) {
            std::string str_i_repr = std::to_string(i);
            size_t needed_spaces_count = max_digits_count - str_i_repr.size();
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
                while (needed_spaces_count-- != 0) {
                    s.push_back(' ');
                }

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
                while (needed_spaces_count-- != 0)
                {
                    s.push_back(' ');
                }

                break;
            }

            s.append(str_i_repr);
            s.push_back(' ');
        }

        // ' ' before '/' is already pushed in the cycle
        s.push_back('/');

        return s;
    }

    friend std::ostream& operator<<(std::ostream& out, const Permutation& permutation) {
        out << permutation.ToString();
        return out;
    }

protected:
    template <class Iterator>
    constexpr void CheckElems(Iterator begin, Iterator end) noexcept(false) {
        for (; begin != end; ++begin) {
            if (!CheckNumber(*begin)) {
                throw std::out_of_range(__PRETTY_FUNCTION__);
            }
        }
    }

    std::vector<uint32_t> elems_;
};

namespace std {
    inline string to_string(const Permutation& permutation) {
        return permutation.ToString();
    }
}

int main() {
    /*
     * Prints out
     *
     * /  1  2  3  4  5  6  7  8  9 10 \
     * \  1  2  3  4  5  6  7  8  9 10 /
     *
     * /  1  2  3  4  5  6  7  8  9 10 \
     * \  2  1  3  4  5  6  7  8 10  9 /
    */

    Permutation p(10);
    std::cout << p << "\n\n";
    p.Swap(1, 2);
    p.Swap(10, 9);
    std::cout << p;
}

