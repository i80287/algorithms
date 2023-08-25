#include <algorithm>      // std::stable_sort
#include <cassert>        // assert
#include <cctype>         // std::isupper, std::isalpha, std::tolower
#include <cmath>          // std::log
#include <cstdint>        // size_t, uint64_t
#include <cstring>        // strcasecmp, _stricmp
#include <ranges>         // std::ranges
#include <string>         // std::string
#include <string_view>    // std::string_view
#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_multiset
#include <vector>         // std::vector

#include "search_lib.hpp" // search_lib::Search

namespace search_lib {
    struct StringViewHasherAndComparer {
#if __cplusplus >= 202002L
        constexpr
#endif
        inline bool operator()(std::string_view lhs, std::string_view rhs) const {
#if __cplusplus >= 202002L
            auto to_lower{ std::ranges::views::transform(static_cast<int(*)(int)>(std::tolower)) };
            return std::ranges::equal(lhs | to_lower, rhs | to_lower);
#else
#if defined(_WIN32) || defined(_WIN64)
            return strcasecmp(lhs.data(), rhs.data()) == 0;
#else
            return _stricmp(lhs.data(), rhs.data()) == 0;
#endif
#endif
        }

        inline std::size_t operator()(std::string_view key_sv) const {
            bool contains_upper = false;
            size_t first_upper_index = 0;
            size_t i = 0;
            for (int c : key_sv) {
                if (std::isupper(c)) {
                    contains_upper = true;
                    first_upper_index = i;
                    break;
                }
                ++i;
            }

            if (!contains_upper) {
                return std::hash<std::string_view>()(key_sv);
            }

            std::string copy(key_sv);
            for (auto word_it = copy.begin() + first_upper_index, end = copy.end();
                word_it != end;
                ++word_it) {
                *word_it = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(*word_it)));
            }

            return std::hash<std::string_view>()(copy);
        }
    };

    using QueryWords = std::vector<string_view>;
    using WordsSet = std::unordered_multiset<string_view, StringViewHasherAndComparer, StringViewHasherAndComparer>;

    struct LineInfo {
        WordsSet Words;
        string_view Line;
        double Score;
    };
    using TextLines = std::vector<LineInfo>;

    namespace text_parse_tools {
        static constexpr bool IsLineSplitSymbol(int c) noexcept {
            return (c == '\n') | (c == '\r');
        }

        static constexpr size_t FirstAlpha(const char* & it) noexcept {
            const char* begin = it;
            for (unsigned char c = static_cast<unsigned char>(*it);
                (!std::isalpha(c)) & (c != '\0');
                c = static_cast<unsigned char>(*++it)) {
            }

            return static_cast<size_t>(it - begin);
        }

        static constexpr size_t FindNextSepDist(const char* & it) noexcept {
            if (*it != '\0') {
                const char* begin = it;
                while (std::isalpha(static_cast<unsigned char>(*++it))) {
                }

                return static_cast<size_t>(it - begin);
            }

            return 0;
        }

        static void SplitTextToWords(string_view text, TextLines& text_lines) {
            constexpr size_t MISSING = static_cast<size_t>(-1);
            search_lib::WordsSet current_line_words;
            const char* it = text.begin();
            size_t line_start = FirstAlpha(it);
            for (size_t length = text.length(), l = line_start, r = l + FindNextSepDist(it);;) {
                if (l < r) {
                    assert(r <= length);
                    current_line_words.insert(text.substr(l, r - l));

                    if (line_start == MISSING) {
                        line_start = l;
                    }
                }

                if (r < length) {
                    if (IsLineSplitSymbol(text[r]) && !current_line_words.empty()) {
                        text_lines.emplace_back(std::move(current_line_words), text.substr(line_start, r - line_start), 0.0);
                        current_line_words.clear();
                        line_start = MISSING;
                    }

                    l = r + 1;
                    r += FindNextSepDist(it);
                } else {
                    break;
                }
            }

            if (!current_line_words.empty()) {
                assert(line_start != MISSING);
                text_lines.emplace_back(std::move(current_line_words), text.substr(line_start), 0.0);
            }
        }

        static void SplitQueryLineToWords(string_view line, search_lib::QueryWords& line_words) {
            const char* it = line.begin();
            for (size_t length = line.length(), l = FirstAlpha(it), r = l + FindNextSepDist(it);;) {
                if (l < r) {
                    assert(r <= length);
                    line_words.push_back(line.substr(l, r - l));
                }

                if (r < length) {
                    l = r + 1;
                    r += FindNextSepDist(it);
                } else {
                    break;
                }
            }
        }
    } // namespace text_parse_tools

    std::vector<string_view> Search(string_view text, string_view query, uint64_t result_size) {
        TextLines text_lines;
        text_parse_tools::SplitTextToWords(text, text_lines);

        QueryWords query_words;
        text_parse_tools::SplitQueryLineToWords(query, query_words);

        std::vector<double> query_words_idf_log;
        query_words_idf_log.reserve(query_words.size());
        auto total_lines_log = std::log(static_cast<double>(text_lines.size()));
        for (string_view query_word : query_words) {
            size_t count = 0;
            for (const auto& line : text_lines) {
                count += line.Words.contains(query_word);
            }

            double word_idf_log = count != 0 ? total_lines_log - std::log(static_cast<double>(count)) : 0;
            query_words_idf_log.push_back(word_idf_log);
        }
        assert(query_words.size() == query_words_idf_log.size());

        for (LineInfo& line : text_lines) {
            double line_score = 0;
            assert(!line.Words.empty());
            double line_words_count = static_cast<double>(line.Words.size());

            for (size_t i = 0, length = query_words.size(); i != length; ++i) {
                if (double q_word_log = query_words_idf_log[i]) {
                    line_score += static_cast<double>(line.Words.count(query_words[i])) / line_words_count * q_word_log;
                }
            }

            line.Score = line_score;
        }

        std::stable_sort(
            text_lines.begin(),
            text_lines.end(),
            [](TextLines::const_reference first_pair, TextLines::const_reference second_pair) noexcept -> bool {
                return first_pair.Score >= second_pair.Score;
        });

        result_size = std::min(text_lines.size(), result_size);
        std::vector<string_view> result_lines;
        result_lines.reserve(result_size);
        for (const LineInfo& line : text_lines) {
            if (result_size-- != 0 && line.Score > 0) {
                result_lines.push_back(line.Line);
            }
            else {
                break;
            }
        }

        return result_lines;
    }
} // namespace search_lib
