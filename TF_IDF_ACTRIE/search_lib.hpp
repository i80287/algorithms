#ifndef _SEARCH_LIB_H_
#define _SEARCH_LIB_H_ 1

#include <cstdint>     // std::size_t
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace search_lib {
    using std::string_view;

    template <bool IsExactWordsMatching = true>
    std::vector<string_view> Search(string_view text, string_view query, std::size_t result_size);
} // namespace search_lib

#endif
