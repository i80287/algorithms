#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "search_lib.hpp"

#ifdef NDEBUG
#error("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

namespace {

using namespace std::literals::string_view_literals;

void test1() {
    constexpr auto text =
        "Lorem Ipsum is simply dummy text\n"
        "of the printing and typesetting industry.\n"
        "Lorem Ipsum has been the industry's standard\n"
        "dummy text ever since the 1500s, when an unknown\n"
        "printer took a galley of type and scrambled it\n"
        "to make a type specimen book. It has survived\n"
        "not only five centuries, but also the leap into\n"
        "electronic typesetting, remaining essentially\n"
        "unchanged. It was popularised in the 1960s with\n"
        "the release of Letraset sheets containing Lorem\n"
        "psum passages, and more recently with desktop\n"
        "publishing software like Aldus PageMaker\n"
        "including versions of Lorem Ipsum."sv;

    constexpr auto query              = "typesetting release"sv;
    constexpr std::size_t result_size = 3;

    auto res = search_lib::Search(text, query, result_size);
    assert((res == std::vector{
                       "electronic typesetting, remaining essentially"sv,
                       "the release of Letraset sheets containing Lorem"sv,
                       "of the printing and typesetting industry."sv,
                   }));
}

void test2() {
    std::string text = []() {
        std::ostringstream buffer;
        {
            std::ifstream fin("Anglo_Saxon_Chronicle.txt");
            fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            buffer << fin.rdbuf();
        }
        return std::move(buffer).str();
    }();
    text.shrink_to_fit();

    constexpr auto query              = "london city borough burg"sv;
    constexpr std::size_t result_size = 32;

    const auto start                  = std::chrono::high_resolution_clock::now();
    std::vector<std::string_view> res = search_lib::Search(text, query, result_size);
    const auto end                    = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    constexpr bool kPrintResults = false;
    if constexpr (kPrintResults) {
        std::cout << "Test 2\nText length: " << text.size() << "\nFound " << res.size()
                  << " lines in " << duration.count() << " ns :\n\n";
        for (std::string_view line : res) {
            std::cout << line << '\n';
        }
    }

    assert((res == std::vector{
                       "          the ancient city;"sv,
                       "(Penguin Classics, London, 1955, 1968)."sv,
                       "          in the fated city,"sv,
                       "was Gilden-borough become a wretched borough.  The monks then"sv,
                       "consternation fled to London."sv,
                       "from London to Canterbury.))"sv,
                       "returned to London, and beset the city without, and fought"sv,
                       "that was afterwards called Peter-borough.  Afterwards came"sv,
                       "The same year also King Alfred fortified the city of London; and"sv,
                       "the Lea, twenty miles above the city of London.   Then, in the"sv,
                       "episcopal see in the city of Antioch."sv,
                       "809 North 1500 West, Salt Lake City, UT 84116, (801) 596-1887, email"sv,
                       "toward London after the earls.  When they came to London, there"sv,
                       "fought against the city of London; but glory be to God, that it"sv,
                       "proceeded; and went then to London.))"sv,
                       "to London; but the people of London attempted to take her, and"sv,
                       "     Salisbury, which was not then in existence; the borough of"sv,
                       "borough, and ordered it to be repaired, and manned both with"sv,
                       "harvest, to Thelwall; and ordered the borough to be repaired, and"sv,
                       "other did before him, nor any after him.  After, Golden-borough"sv,
                       "became a wretched borough.  Then chose the monks for abbot Brand"sv,
                       "came to his bishopric, almost all the borough of Lincoln was"sv,
                       "London, 1953, 1972).  HIGHLY RECOMMENDED.  Contains side-by-side"sv,
                       "midsummer.  Then rode he to the borough of Tamworth; and all the"sv,
                       "Golden-borough.  Then it waxed greatly, in land, and in gold, and"sv,
                       "her into Oxford, and gave her the borough.  When the king was"sv,
                       "within the city of Canterbury.  Bertwald, who before this was"sv,
                       "one part east, another to Rochester.  This city they surrounded,"sv,
                       "Seine, fixed their winter-quarters in the city of Paris. (37)"sv,
                       "delivered the city to them, whose life Archbishop Elfeah formerly"sv,
                       "bridge.  Afterwards they trenched the city without, so that no"sv,
                       "there continually within the city as long as she could."sv,
                   }));
}

}  // namespace

int main() {
    test1();
    test2();
}
