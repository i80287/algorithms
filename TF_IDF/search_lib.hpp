#include <cstdint>     // uint64_t
#include <string_view> // std::string_view
#include <vector>      // std::vector

namespace search_lib {
    using std::string_view;

    std::vector<string_view> Search(string_view text, string_view query, uint64_t result_size);
} // namespace search_lib
