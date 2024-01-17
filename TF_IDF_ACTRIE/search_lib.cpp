#include <algorithm>      // std::stable_sort
#include <cmath>          // std::log
#include <cstdint>        // size_t, uint32_t
#include <string_view>    // std::string_view
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

#include "actrie.hpp"     // ACTrieADS::ACTrie
#include "search_lib.hpp" // search_lib::Search

namespace search_lib {
    using ACTrie = ACTrieADS::ACTrie<'a', 'z', true>;

    static inline size_t ParseQuery(string_view query, ACTrie& act) {
        constexpr char QUERY_WORDS_DELIMITER = ' ';
        size_t query_words_count = !query.empty();
        for (char c : query) {
            query_words_count += c == QUERY_WORDS_DELIMITER;
        }

        act.ReservePlaceForPatterns(query_words_count);

        size_t nonempty_query_words_count = 0;
        size_t prev_delim_index = static_cast<size_t>(-1);
        for (size_t i = 0, query_size = query.size(); i < query_size; i++) {
            if (query[i] == QUERY_WORDS_DELIMITER) {
                string_view slice(query.data() + prev_delim_index + 1, i - prev_delim_index - 1);
                if (!slice.empty()) {
                    act.AddPattern(slice);
                    nonempty_query_words_count++;
                }

                prev_delim_index = i;
            }
        }

        string_view slice(query.data() + prev_delim_index + 1, query.size() - (prev_delim_index + 1));
        if (!slice.empty()) {
            act.AddPattern(slice);
            nonempty_query_words_count++;
        }

        return nonempty_query_words_count;
    }

    template <bool IsExactWordsMatching>
    std::vector<string_view> Search(string_view text, string_view query, size_t result_size) {
        ACTrie act;
        size_t query_words_count = ParseQuery(query, act);

        act.ComputeLinks();

        struct LineInfo {
            size_t line_number = 0;
            size_t words_count = 0;
            size_t line_start_index = 0;
            size_t line_end_index = 0;
            std::unordered_map<size_t, uint32_t> query_words_indexes;
        };

        std::vector<LineInfo> query_words_on_lines;

        size_t total_lines = act.RunTextCountLines<IsExactWordsMatching>(text,
            [&query_words_on_lines](size_t line_number, size_t query_word_index) constexpr {
                if (query_words_on_lines.empty() || query_words_on_lines.back().line_number != line_number) {
                    query_words_on_lines.emplace_back().line_number = line_number;
                }

                ++query_words_on_lines.back().query_words_indexes[query_word_index];
            },
            [&query_words_on_lines](size_t line_number, size_t words_on_current_line, size_t line_start_index, size_t line_end_index) constexpr noexcept {
                if (query_words_on_lines.empty()) {
                    return;
                }

                LineInfo& ref = query_words_on_lines.back();
                if (ref.line_number != line_number) {
                    return;
                }

                ref.words_count = words_on_current_line;
                ref.line_start_index = line_start_index;
                ref.line_end_index = line_end_index;
            });

        std::vector<double> query_words_inv_idf_log(query_words_count);
        double total_lines_log = std::log(total_lines);
        for (size_t query_word_index = 0; query_word_index < query_words_count; ++query_word_index) {
            size_t count = 0;
            for (const LineInfo& line_info : query_words_on_lines) {
                count += line_info.query_words_indexes.contains(query_word_index);
            }

            double word_idf_log = count != 0 ? total_lines_log - std::log(count) : 0; // log(total_lines / count);
            query_words_inv_idf_log[query_word_index] = word_idf_log;
        }

        std::vector<std::pair<double, size_t>> lines_score;
        lines_score.reserve(query_words_on_lines.size());

        size_t line_index = 0;
        for (const LineInfo& line_info : query_words_on_lines) {
            double line_score = 0;
            double words_count = static_cast<double>(line_info.words_count);
            for (auto &&[query_word_index, word_count] : line_info.query_words_indexes) {
                line_score += (word_count / words_count) * query_words_inv_idf_log.at(query_word_index);
            }

            lines_score.emplace_back(line_score, line_index);
            line_index++;
        }

        std::stable_sort(lines_score.begin(),
            lines_score.end(),
            [](const std::pair<double, size_t>& p1, const auto& p2) constexpr noexcept -> bool {
                return p1.first >= p2.first;
            });

        result_size = std::min(lines_score.size(), result_size);
        std::vector<string_view> result_lines;
        result_lines.reserve(result_size);

        for (auto iter = lines_score.cbegin(), iter_end = iter + ptrdiff_t(result_size); iter != iter_end; ++iter) {
            line_index = iter->second;
            size_t l = query_words_on_lines[line_index].line_start_index;
            size_t r = query_words_on_lines[line_index].line_end_index;
            result_lines.emplace_back(std::string_view(text.data() + l, r - l));
        }

        return result_lines;
    }

    template std::vector<string_view> Search<true>(string_view text, string_view query, size_t result_size);
    template std::vector<string_view> Search<false>(string_view text, string_view query, size_t result_size);
} // namespace search_lib
