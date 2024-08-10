#pragma once

#ifdef NDEBUG
#warning("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

#include <array>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "config_macros.hpp"
#if CONFIG_HAS_INCLUDE(<source_location>)
#include <source_location>
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

}  // namespace test_tools_detail

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L

inline void throw_if_not(bool expr, const char* message_format,
                         const std::source_location src = std::source_location::current()) {
    if (unlikely(!expr)) {
        ::test_tools::test_tools_detail::throw_impl(message_format, src.file_name(), src.line(),
                                                    src.function_name());
    }
}

ATTRIBUTE_ALWAYS_INLINE inline void log_tests_started(
    const std::source_location src = std::source_location::current()) noexcept {
    printf("Started tests in %s\n", src.function_name());
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

#endif

struct Wrapper final {
    FILE* const file;
    Wrapper(const char* fname, const char* mode) : file(DoFOpenOrThrow(fname, mode)) {}
    Wrapper(const Wrapper&)            = delete;
    Wrapper(Wrapper&&)                 = delete;
    Wrapper& operator=(const Wrapper&) = delete;
    Wrapper& operator=(Wrapper&&)      = delete;
    ~Wrapper() {
        std::fclose(file);
    }

private:
    ATTRIBUTE_RETURNS_NONNULL static std::FILE* DoFOpenOrThrow(const char* fname,
                                                               const char* mode) {
        std::FILE* const file_handle = std::fopen(fname, mode);
#if CONFIG_HAS_AT_LEAST_CXX_20
        if (file_handle == nullptr) [[unlikely]]
#else
        if (unlikely(file_handle == nullptr))
#endif
        {
            ThrowOnFOpenFail(fname, mode);
        }

        return file_handle;
    }
    [[noreturn]] ATTRIBUTE_COLD static void ThrowOnFOpenFail(const char* fname, const char* mode) {
        const auto errno_value = errno;
        std::array<char, 1024> buffer{};
        int bytes_written = std::snprintf(buffer.data(), buffer.size(),
                                          "Wrapper::Wrapper(const char* fname, const char* mode): "
                                          "std::fopen(\"%s\", \"%s\") failed: %s",
                                          fname, mode, std::strerror(errno_value));
#if CONFIG_HAS_AT_LEAST_CXX_20
        if (bytes_written < 0) [[unlikely]]
#else
        if (bytes_written < 0)
#endif
        {
            constexpr std::string_view msg =
                "Wrapper::Wrapper(const char* fname,const char* mode): "
                "std::snprintf failed after std::fopen failed";
            static_assert(msg.size() <= std::size(buffer),
                          "Wrapper::Wrapper(const char*,const char*)");
            std::char_traits<char>::copy(buffer.data(), msg.data(), msg.size());
        }
        throw std::runtime_error(buffer.data());
    }
};

}  // namespace test_tools
