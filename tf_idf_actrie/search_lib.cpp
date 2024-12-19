#include "search_lib.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "actrie.hpp"

namespace search_lib {

using std::size_t;
using std::uint32_t;

namespace {

using ACTrieBuilder = actrie::ACTrieBuilder<'a', 'z', /*IsCaseInsensetive = */ true>;
using ACTrie        = typename ACTrieBuilder::ACTrieType;

template <char QueryWordsDelimiter = ' '>
ACTrie ParseQuery(string_view query) {
    const size_t query_words_count = std::accumulate(
        query.begin(), query.end(), query.empty() ? size_t{0} : size_t{1},
        [](size_t current_count, char c) constexpr noexcept -> size_t {
            return current_count + (c == QueryWordsDelimiter ? size_t{1} : size_t{0});
        });

    auto act_builder = ACTrieBuilder::WithCapacity(query_words_count);

    size_t prev_delim_index = static_cast<size_t>(-1);
    for (size_t i = 0, query_size = query.size(); i < query_size; i++) {
        if (query[i] == QueryWordsDelimiter) {
            const string_view slice(query.data() + (prev_delim_index + 1),
                                    i - (prev_delim_index + 1));
            if (!slice.empty()) {
                act_builder.AddPattern(slice);
            }

            prev_delim_index = i;
        }
    }

    const string_view slice(query.data() + (prev_delim_index + 1),
                            query.size() - (prev_delim_index + 1));
    if (!slice.empty()) {
        act_builder.AddPattern(slice);
    }

    return std::move(act_builder).Build();
}

}  // namespace

template <bool IsExactWordsMatching>
std::vector<string_view> Search(string_view text, string_view query, size_t max_result_size) {
    const ACTrie act               = ParseQuery(query);
    const size_t query_words_count = act.PatternsSize();

    struct LineInfo {
        size_t LineNumber     = 0;
        size_t WordsCount     = 0;
        size_t LineStartIndex = 0;
        size_t LineEndIndex   = 0;
        std::unordered_map<size_t, uint32_t> QueryWordsIndexes{};
    };

    std::vector<LineInfo> query_words_on_lines;

    const size_t total_lines =
        act.FindAllSubstringsInTextAndCountLines</* IsExactWordsMatching = */ IsExactWordsMatching,
                                                 /* CountEmptyLines = */ false>(
            text,
            [&query_words_on_lines](size_t line_number, size_t query_word_index) constexpr {
                const bool new_line = query_words_on_lines.empty() ||
                                      query_words_on_lines.back().LineNumber != line_number;
                if (new_line) {
                    query_words_on_lines.emplace_back().LineNumber = line_number;
                }
                ++query_words_on_lines.back().QueryWordsIndexes[query_word_index];
            },
            [&query_words_on_lines](size_t line_number, size_t words_on_current_line,
                                    size_t line_start_index,
                                    size_t line_end_index) constexpr noexcept {
                if (query_words_on_lines.empty()) {
                    return;
                }

                LineInfo& ref = query_words_on_lines.back();
                assert(ref.LineNumber <= line_number);
                if (ref.LineNumber != line_number) {
                    return;
                }

                ref.WordsCount     = words_on_current_line;
                ref.LineStartIndex = line_start_index;
                ref.LineEndIndex   = line_end_index;
            });

    std::vector<double> query_words_inv_idf_log(query_words_count);
    const double total_lines_log = std::log(static_cast<double>(total_lines));
    for (size_t query_word_index = 0; query_word_index < query_words_count; ++query_word_index) {
        const size_t count = std::accumulate(
            query_words_on_lines.begin(), query_words_on_lines.end(), size_t{0},
            [query_word_index](size_t current_count, const LineInfo& line_info) noexcept {
                return current_count + line_info.QueryWordsIndexes.contains(query_word_index);
            });

        // log(total_lines / count);
        const double word_idf_log =
            count > 0 ? total_lines_log - std::log(static_cast<double>(count)) : 0;
        query_words_inv_idf_log[query_word_index] = word_idf_log;
    }

    struct LineScoreWithIndex {
        double Score{};
        size_t LineIndex{};
    };
    std::vector<LineScoreWithIndex> lines_score(query_words_on_lines.size());
    for (size_t line_index = 0; const LineInfo& line_info : query_words_on_lines) {
        double line_score        = 0;
        const double words_count = static_cast<double>(line_info.WordsCount);
        for (auto&& [query_word_index, word_count] : line_info.QueryWordsIndexes) {
            line_score += (word_count / words_count) * query_words_inv_idf_log.at(query_word_index);
        }

        lines_score[line_index] = LineScoreWithIndex{
            .Score     = line_score,
            .LineIndex = line_index,
        };
        line_index++;
    }

    std::stable_sort(
        lines_score.begin(), lines_score.end(),
        [](const LineScoreWithIndex& p1, const LineScoreWithIndex& p2) constexpr noexcept {
            return p1.Score > p2.Score;
        });

    const size_t result_size = std::min(lines_score.size(), max_result_size);
    std::vector<string_view> result_lines(result_size);
    for (size_t result_index = 0; result_index < result_size; result_index++) {
        const size_t line_index       = lines_score[result_index].LineIndex;
        const size_t line_start_index = query_words_on_lines[line_index].LineStartIndex;
        const size_t line_end_index   = query_words_on_lines[line_index].LineEndIndex;
        result_lines[result_index] =
            text.substr(line_start_index, line_end_index - line_start_index);
    }

    return result_lines;
}

template std::vector<string_view> Search<true>(string_view text,
                                               string_view query,
                                               size_t max_result_size);
template std::vector<string_view> Search<false>(string_view text,
                                                string_view query,
                                                size_t max_result_size);

}  // namespace search_lib
