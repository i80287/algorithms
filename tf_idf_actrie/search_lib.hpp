#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "../misc/config_macros.hpp"

namespace search_lib {

using std::size_t;
using std::string_view;
using std::vector;

static constexpr bool DefaultIsExactWordsMatching = true;

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] vector<string_view> Search(string_view text ATTRIBUTE_LIFETIME_BOUND,
                                         string_view query,
                                         size_t max_result_size);
// At this point ATTRIBUTE_LIFETIME_BOUND doesn't work for the std::string_view yet

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] inline vector<string_view> Search(const std::string& text ATTRIBUTE_LIFETIME_BOUND,
                                                const string_view query,
                                                const size_t max_result_size) {
    return search_lib::Search<IsExactWordsMatching>(string_view{text}, query, max_result_size);
}

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] inline vector<string_view> Search(const char* const text ATTRIBUTE_LIFETIME_BOUND,
                                                const string_view query,
                                                const size_t max_result_size) {
    return search_lib::Search<IsExactWordsMatching>(string_view{text}, query, max_result_size);
}

}  // namespace search_lib
