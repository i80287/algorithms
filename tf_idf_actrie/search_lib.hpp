#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#if defined(__clang__) && defined(__cplusplus) && __cplusplus >= 201703L
#define ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#else
#define ATTRIBUTE_LIFETIME_BOUND
#endif

namespace search_lib {

using std::string_view;

static constexpr bool DefaultIsExactWordsMatching = true;

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] std::vector<string_view> Search(string_view text, string_view query,
                                              std::size_t max_result_size);
// ATTRIBUTE_LIFETIME_BOUND doesn't work for the std::string_view

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] std::vector<string_view> Search(const std::string& text ATTRIBUTE_LIFETIME_BOUND,
                                              string_view query, std::size_t max_result_size) {
    return ::search_lib::Search(std::string_view(text), query, max_result_size);
}

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] std::vector<string_view> Search(const char* text ATTRIBUTE_LIFETIME_BOUND,
                                              string_view query, std::size_t max_result_size) {
    return ::search_lib::Search(std::string_view(text), query, max_result_size);
}

template <bool IsExactWordsMatching = DefaultIsExactWordsMatching>
[[nodiscard]] std::vector<string_view> Search(const char* text ATTRIBUTE_LIFETIME_BOUND,
                                              std::size_t text_size, string_view query,
                                              std::size_t max_result_size) {
    return ::search_lib::Search(std::string_view(text, text_size), query, max_result_size);
}

}  // namespace search_lib
