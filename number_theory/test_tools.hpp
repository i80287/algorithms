#pragma once

#ifdef NDEBUG
#warning "Can't test properly with NDEBUG macro defined (macro won't be undefined manually)"
#endif

#include <array>
#include <cstdio>
#include <stdexcept>

#include "config_macros.hpp"
#if CONFIG_HAS_INCLUDE(<source_location>)
#include <source_location>
#endif

namespace test_tools {
namespace test_tools_detail {

template <class T>
[[noreturn]] ATTRIBUTE_COLD inline void throw_impl(const char* message_format, T arg,
                                                   const char* file_name, uint32_t line,
                                                   const char* function_name) {
    std::array<char, 1024> buffer;
    int bytes_written     = std::snprintf(buffer.data(), buffer.size(),
                                          "Check failed at %s:%u %s\nError messssage: ", file_name,
                                          line, function_name);
    size_t err_msg_offset = uint32_t(bytes_written);
    if (bytes_written < 0 || err_msg_offset > buffer.size()) {
        perror("std::snprintf");
        err_msg_offset = 0;
    }
    bytes_written =
        std::snprintf(&buffer[err_msg_offset], buffer.size() - err_msg_offset, message_format, arg);
    if (bytes_written < 0) {
        perror("std::snprintf");
    }
    throw std::runtime_error(buffer.data());
}

}  // namespace test_tools_detail

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L

template <class T>
inline void throw_if_not(bool expr, const char* message_format, T arg,
                         const std::source_location src = std::source_location::current()) {
    if (unlikely(!expr)) {
        test_tools_detail::throw_impl(message_format, arg, src.file_name(), src.line(),
                                      src.function_name());
    }
}

ATTRIBUTE_ALWAYS_INLINE inline void log_tests_started(
    const std::source_location src = std::source_location::current()) noexcept {
    printf("Started tests in %s\n", src.function_name());
}

#else

namespace test_tools_detail {

template <class T>
inline void throw_if_not_impl(bool expr, const char* message_format, T arg, const char* file_name,
                              std::uint32_t line, const char* function_name) {
    if (unlikely(!expr)) {
        test_tools_detail::throw_impl(message_format, arg, file_name, line, function_name);
    }
}

}  // namespace test_tools_detail

#define throw_if_not(expr, message_format, arg)                                         \
    test_tools_detail::throw_if_not_impl(expr, message_format, arg, __FILE__, __LINE__, \
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
    Wrapper(const char* fname, const char* mode) : file(std::fopen(fname, mode)) {
        if (file == nullptr) [[unlikely]] {
            ThrowOnFOpenFail(fname, mode);
        }
    }
    Wrapper(const Wrapper&)            = delete;
    Wrapper(Wrapper&&)                 = delete;
    Wrapper& operator=(const Wrapper&) = delete;
    Wrapper& operator=(Wrapper&&)      = delete;
    ~Wrapper() {
        std::fclose(file);
    }

private:
    [[noreturn]] ATTRIBUTE_COLD static void ThrowOnFOpenFail(const char* fname, const char* mode) {
        std::array<char, 1024> buffer;
        int bytes_written = std::snprintf(
            buffer.data(), buffer.size(),
            "Wrapper::Wrapper(const char* fname, const char* mode): std::fopen(%s, %s) failed",
            fname, mode);
        if (bytes_written < 0) [[unlikely]] {
            constexpr std::string_view msg =
                "Wrapper::Wrapper(const char* fname,const char* mode): "
                "std::snprintf failed after std::fopen failed";
            static_assert(msg.size() <= std::size(buffer),
                          "Wrapper::Wrapper(const char*,const char*)");
            std::char_traits<char>::copy(buffer.data(), msg.data(), msg.size());
            perror(buffer.data());
        }
        perror(buffer.data());
        throw std::runtime_error(buffer.data());
    }
};

}  // namespace test_tools
