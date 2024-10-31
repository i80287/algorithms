#pragma once

#ifdef NDEBUG
#error("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"
#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L && \
    CONFIG_HAS_INCLUDE(<source_location>)
#define TEST_TOOLS_HAS_SOURCE_LOCATION 1
#include <source_location>
#else
#define TEST_TOOLS_HAS_SOURCE_LOCATION 0
#endif

// https://en.cppreference.com/w/cpp/language/consteval
#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
#define TEST_TOOLS_CONSTEVAL consteval
#else
#define TEST_TOOLS_CONSTEVAL constexpr
#endif

namespace test_tools {
namespace test_tools_detail {

[[noreturn]] ATTRIBUTE_COLD inline void throw_impl(const char* message, const char* file_name,
                                                   std::uint32_t line, const char* function_name) {
    std::array<char, 1024> buffer{};
    const int bytes_written =
        std::snprintf(buffer.data(), buffer.size(), "Check failed at %s:%u %s\nError message: %s\n",
                      file_name, line, function_name, message);
    if (bytes_written < 0) {
        perror("std::snprintf");
        buffer[0] = '\0';
    }
    throw std::runtime_error(buffer.data());
}

inline void log_location_impl(const char* file_name, std::uint32_t line,
                              const char* function_name) noexcept {
    printf("%s:%u: %s\n", file_name, line, function_name);
}

inline void log_message_impl(const char* file_name, std::uint32_t line, const char* function_name,
                             const char* message) noexcept {
    printf("%s:%u: %s:\n    %s\n", file_name, line, function_name, message);
}

inline void log_message_impl(const char* file_name, std::uint32_t line, const char* function_name,
                             std::string_view message) noexcept {
    const auto message_size = static_cast<int>(message.size());
    printf("%s:%u: %s:\n    %.*s\n", file_name, line, function_name, message_size, message.data());
}

}  // namespace test_tools_detail

#if TEST_TOOLS_HAS_SOURCE_LOCATION

inline void throw_if_not(bool expr, const char* message_format,
                         const std::source_location src = std::source_location::current()) {
    if (unlikely(!expr)) {
        ::test_tools::test_tools_detail::throw_impl(message_format, src.file_name(), src.line(),
                                                    src.function_name());
    }
}

ATTRIBUTE_ALWAYS_INLINE inline void log_tests_started(
    const std::source_location src = std::source_location::current()) noexcept {
    int ret = printf("Started tests in %s\n", src.function_name());
    assert(ret > 0);
    ret = fflush(stdout);
    assert(ret == 0);
}

ATTRIBUTE_ALWAYS_INLINE inline void log_location(
    const std::source_location src = std::source_location::current()) noexcept {
    ::test_tools::test_tools_detail::log_location_impl(src.file_name(), src.line(),
                                                       src.function_name());
}

ATTRIBUTE_ALWAYS_INLINE inline void log_message(
    const char* message,
    const std::source_location src = std::source_location::current()) noexcept {
    ::test_tools::test_tools_detail::log_message_impl(src.file_name(), src.line(),
                                                      src.function_name(), message);
}

ATTRIBUTE_ALWAYS_INLINE inline void log_message(
    std::string_view message,
    const std::source_location src = std::source_location::current()) noexcept {
    ::test_tools::test_tools_detail::log_message_impl(src.file_name(), src.line(),
                                                      src.function_name(), message);
}

#else

namespace test_tools_detail {

inline void throw_if_not_impl(bool expr, const char* message_format, const char* file_name,
                              std::uint32_t line, const char* function_name) {
    if (unlikely(!expr)) {
        ::test_tools::test_tools_detail::throw_impl(message_format, file_name, line, function_name);
    }
}

}  // namespace test_tools_detail

#define throw_if_not(expr, message_format)                                                       \
    ::test_tools::test_tools_detail::throw_if_not_impl(expr, message_format, __FILE__, __LINE__, \
                                                       FUNCTION_MACRO)

namespace test_tools_detail {

ATTRIBUTE_ALWAYS_INLINE inline void log_tests_started_impl(const char* function_name) noexcept {
    printf("Started tests in %s\n", function_name);
}

}  // namespace test_tools_detail

#define log_tests_started() test_tools_detail::log_tests_started_impl(FUNCTION_MACRO)

#define log_location() test_tools_detail::log_location_impl(__FILE__, __LINE__, FUNCTION_MACRO)

#define log_message(message) \
    test_tools_detail::log_message_impl(__FILE__, __LINE__, FUNCTION_MACRO, message);

#endif

struct FilePtr final {
    using FileHandle = std::FILE*;
    FileHandle const file;  // NOLINT(misc-misplaced-const)

    [[nodiscard]] constexpr operator FileHandle() noexcept {
        return file;
    }
    [[nodiscard]] constexpr std::FILE* operator->() noexcept {
        return file;
    }

    FilePtr(const char* fname, const char* mode) : file(DoFOpenOrThrow(fname, mode)) {}
    FilePtr(const FilePtr&)            = delete;
    FilePtr(FilePtr&&)                 = delete;
    FilePtr& operator=(const FilePtr&) = delete;
    FilePtr& operator=(FilePtr&&)      = delete;
    ~FilePtr() {
        if (std::fclose(file) != 0) {
            std::perror("fclose");
        }
    }

private:
    ATTRIBUTE_RETURNS_NONNULL ATTRIBUTE_ALWAYS_INLINE static FileHandle DoFOpenOrThrow(
        const char* fname, const char* mode) {
        // NOLINTNEXTLINE(misc-misplaced-const)
        FileHandle const file_handle = std::fopen(fname, mode);
        if (unlikely(file_handle == nullptr)) {
            ThrowOnFOpenFail(fname, mode);
        }

        return file_handle;
    }
    [[noreturn]] ATTRIBUTE_COLD static void ThrowOnFOpenFail(const char* fname, const char* mode) {
        const auto errno_value = errno;
        std::array<char, 1024> buffer{};
        int bytes_written = std::snprintf(buffer.data(), buffer.size(),
                                          "FilePtr::FilePtr(const char* fname, const char* mode): "
                                          "std::fopen(\"%s\", \"%s\") failed: %s",
                                          fname, mode, std::strerror(errno_value));
        if (bytes_written < 0) {
            constexpr std::string_view msg =
                "FilePtr::FilePtr(const char* fname,const char* mode): "
                "std::snprintf failed after std::fopen failed";
            static_assert(msg.size() < std::size(buffer),
                          "FilePtr::FilePtr(const char*,const char*)");
            std::char_traits<char>::copy(buffer.data(), msg.data(), msg.size());
            buffer[msg.size()] = '\0';
        }

        throw std::runtime_error(buffer.data());
    }
};

namespace test_tools_detail {

TEST_TOOLS_CONSTEVAL std::size_t get_typename_end_pos_impl(const std::string_view s) {
    // Variables are not inside of the for init for
    //  the compatibility with C++17.
    std::size_t opened_square_brackets   = 0;
    std::size_t opened_round_brackets    = 0;
    std::size_t opened_curly_brackets    = 0;
    std::size_t opened_triangle_brackets = 0;
    std::size_t i                        = 0;
    for (const char c : s) {
        switch (static_cast<std::uint8_t>(c)) {
            case '(': {
                opened_round_brackets++;
                break;
            }
            case ')': {
                if (opened_round_brackets == 0) {
                    return i;
                }
                opened_round_brackets--;
                break;
            }
            case '{': {
                opened_curly_brackets++;
                break;
            }
            case '}': {
                if (opened_round_brackets == 0) {
                    return i;
                }
                opened_round_brackets--;
                break;
            }
            case '[': {
                opened_square_brackets++;
                break;
            }
            case ']': {
                if (opened_square_brackets == 0) {
                    return i;
                }
                opened_square_brackets--;
                break;
            }
            case '<': {
                opened_triangle_brackets++;
                break;
            }
            case '>': {
                if (opened_triangle_brackets == 0) {
                    return i;
                }
                opened_triangle_brackets--;
                break;
            }
            case ',':
            case ';': {
                if (opened_square_brackets == 0 && opened_round_brackets == 0 &&
                    opened_curly_brackets == 0 && opened_triangle_brackets == 0) {
                    return i;
                }
                break;
            }
            default:
                break;
        }

        i++;
    }

    return s.size();
}

#define CONSTEVAL_ASSERT_CONCAT_IMPL(arg1, arg2, arg3) arg1##arg2##arg3
#define CONSTEVAL_ASSERT_CONCAT(arg1, arg2, arg3)      CONSTEVAL_ASSERT_CONCAT_IMPL(arg1, arg2, arg3)
#define CONSTEVAL_ASSERT_GENERATE_UNIQUE_NAME(prefix) \
    CONSTEVAL_ASSERT_CONCAT(prefix, _unique_addendum_, __COUNTER__)

#define CONSTEVAL_ASSERT(expr)                                                    \
    do {                                                                          \
        [[maybe_unused]] const int CONSTEVAL_ASSERT_GENERATE_UNIQUE_NAME(guard) = \
            bool{expr} ? 0 : throw 0;                                \
    } while (false)

TEST_TOOLS_CONSTEVAL std::string_view get_typename_impl(const std::string_view function_name) {
    const auto is_space = [](char c) constexpr noexcept {
        switch (static_cast<std::uint8_t>(c)) {
            case '\n':
            case '\v':
            case '\f':
            case '\r':
            case ' ':
                return true;
            default:
                return false;
        }
    };

#if defined(__GNUG__) || defined(__clang__)
    constexpr std::string_view type_prefix = "T = ";
    const auto prefix_start_pos            = function_name.find(type_prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != std::string_view::npos);
    auto typename_start_pos = prefix_start_pos + type_prefix.size();
#elif defined(_MSC_VER)
    constexpr std::string_view type_prefix = "get_typename<";
    const auto prefix_start_pos            = function_name.find(type_prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != std::string_view::npos);
    auto typename_start_pos = prefix_start_pos + type_prefix.size();
    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    std::string_view piece                 = function_name.substr(typename_start_pos);
    constexpr std::string_view kKeywords[] = {
        "class",
        "struct",
        "enum",
        "union",
    };
    for (bool continue_cut = true; continue_cut;) {
        continue_cut = false;
        while (is_space(piece.front())) {
            piece.remove_prefix(1);
            typename_start_pos++;
        }
        for (const auto keyword : kKeywords) {
#if CONFIG_HAS_AT_LEAST_CXX_20
            if (piece.starts_with(keyword))
#else
            if (keyword.size() <= piece.size() && keyword == piece.substr(0, keyword.size()))
#endif
            {
                piece.remove_prefix(keyword.size());
                typename_start_pos += keyword.size();
                continue_cut = true;
                break;
            }
        }
    }

#else
// cppcheck-suppress [preprocessorErrorDirective]
#error("Unsupported compiler")
#endif

    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    while (is_space(function_name[typename_start_pos])) {
        typename_start_pos++;
    }
    CONSTEVAL_ASSERT(typename_start_pos < function_name.size());
    const auto typename_end_pos =
        typename_start_pos + get_typename_end_pos_impl(function_name.substr(typename_start_pos));
    CONSTEVAL_ASSERT(typename_end_pos < function_name.size());
    CONSTEVAL_ASSERT(typename_start_pos < typename_end_pos);
    return function_name.substr(typename_start_pos, typename_end_pos - typename_start_pos);
}

}  // namespace test_tools_detail

template <class T>
[[nodiscard]] TEST_TOOLS_CONSTEVAL std::string_view get_typename() {
    const std::string_view function_name =
#if TEST_TOOLS_HAS_SOURCE_LOCATION
        std::source_location::current().function_name();
#else
        FUNCTION_MACRO;
#endif
    return ::test_tools::test_tools_detail::get_typename_impl(function_name);
}

namespace test_tools_detail {

template <class EnumType, EnumType EnumValue>
TEST_TOOLS_CONSTEVAL std::string_view get_enum_value_name_impl(
    const std::string_view function_name) {
    using std::string_view;

#if defined(__GNUG__) || defined(__clang__)
    constexpr string_view prefix = "EnumValue = ";
#elif defined(_MSC_VER)
    constexpr string_view prefix = "get_enum_value_name<";
#else
// cppcheck-suppress [preprocessorErrorDirective]
#error("Unsupported compiler")
#endif

    const auto prefix_start_pos = function_name.find(prefix);
    CONSTEVAL_ASSERT(prefix_start_pos != string_view::npos);
    auto value_start_pos = prefix_start_pos + prefix.size();
    CONSTEVAL_ASSERT(value_start_pos < function_name.size());
    const auto value_end_pos =
        value_start_pos + get_typename_end_pos_impl(function_name.substr(value_start_pos));
    CONSTEVAL_ASSERT(value_start_pos < value_end_pos);
    CONSTEVAL_ASSERT(value_end_pos < function_name.size());
    std::string_view full_name =
        function_name.substr(value_start_pos, value_end_pos - value_start_pos);
    constexpr std::string_view kScopeResolutionOperator = "::";
    if (const auto scope_res_operator_pos = full_name.rfind(kScopeResolutionOperator);
        scope_res_operator_pos != std::string_view::npos) {
        full_name = full_name.substr(scope_res_operator_pos + kScopeResolutionOperator.size());
    }
    return full_name;
}

#undef CONSTEVAL_ASSERT
#undef CONSTEVAL_ASSERT_GENERATE_UNIQUE_NAME
#undef CONSTEVAL_ASSERT_CONCAT
#undef CONSTEVAL_ASSERT_CONCAT_IMPL

}  // namespace test_tools_detail

template <auto EnumValue, class EnumType = decltype(EnumValue)>
#if __cplusplus >= 202002L
    requires(std::is_enum_v<EnumType>)
#endif
[[nodiscard]] TEST_TOOLS_CONSTEVAL std::string_view get_enum_value_name() {
    const std::string_view function_name =
#if TEST_TOOLS_HAS_SOURCE_LOCATION
        std::source_location::current().function_name();
#else
        FUNCTION_MACRO;
#endif
    return ::test_tools::test_tools_detail::get_enum_value_name_impl<EnumType, EnumValue>(
        function_name);
}

template <class Observed = void>
struct EchoLogger {
    EchoLogger() {
        ::test_tools::log_location();
    }
    EchoLogger(const EchoLogger&) noexcept {
        ::test_tools::log_location();
    }
    EchoLogger(EchoLogger&&) noexcept {
        ::test_tools::log_location();
    }
    EchoLogger& operator=(const EchoLogger&) noexcept {
        ::test_tools::log_location();
        return *this;
    }
    EchoLogger& operator=(EchoLogger&&) noexcept {
        ::test_tools::log_location();
        return *this;
    }
    ~EchoLogger() {
        ::test_tools::log_location();
    }
};

}  // namespace test_tools

#undef TEST_TOOLS_CONSTEVAL
#undef TEST_TOOLS_HAS_SOURCE_LOCATION
