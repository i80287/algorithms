#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace search_lib {
    using std::string_view;

    template <bool IsExactWordsMatching = true>
    std::vector<string_view> Search(string_view text, string_view query, std::size_t result_size);
} // namespace search_lib
