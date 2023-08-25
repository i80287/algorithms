#include <chrono>         // std::chrono
#include <cstdio>         // std::FILE, std::fopen, std::fclose
#include <iostream>       // std::cin, std::cout
#include <string>         // std::string

#include "search_lib.hpp" // search_lib::Search

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

    constexpr auto query = "typesetting release"sv;
    constexpr size_t result_size = 3;

    auto res = search_lib::Search(text, query, result_size);
    std::cout << "Test 1\n";
    for (auto line : res) {
        std::cout << line << '\n';
    }
}

void test2() {
    constexpr const char* filename = "Anglo_Saxon_Chronicle.txt";
    std::FILE* file = std::fopen(filename, "r");
    if (file == nullptr) {
        std::clog << "Was not able to open file\n";
        return;
    }

    std::string text;
    text.reserve(1 << 20);
    char buffer[1024] {};
    while (fgets(buffer, sizeof(buffer) * sizeof(buffer[0]), file) != nullptr) {
        text += buffer;
    }

    if (std::fclose(file)) {
        std::clog << "An error occured while closing file " << filename << '\n';
    }

    text.shrink_to_fit();

    constexpr auto query = "london city borough burg"sv;
    constexpr size_t result_size = 32;

    auto start = std::chrono::steady_clock::now();
    auto res = search_lib::Search(text, query, result_size);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Test 2\nText length: " << text.length() << "\nFound " << res.size() << " lines in " << duration << ":\n\n";
    for (auto line : res) {
        std::cout << line << '\n';
    }
}

int main() {
    test1();
    test2();
}
