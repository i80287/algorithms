#pragma once

#ifdef NDEBUG
#error("Can't test properly with NDEBUG macro defined (macro won't be undefined manually)")
#endif

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

#include "../misc/config_macros.hpp"

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L && \
    CONFIG_HAS_INCLUDE(<source_location>)
#define TEST_TOOLS_HAS_SOURCE_LOCATION
#include <source_location>
#endif

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

namespace test_tools {

namespace test_tools_detail {

[[noreturn]] ATTRIBUTE_COLD inline void throw_impl(const char* const message,
                                                   const char* const file_name,
                                                   const std::uint32_t line,
                                                   const char* const function_name) {
    constexpr std::size_t kMaxErrorMessageSize = 1024 * sizeof(char);
    std::array<char, kMaxErrorMessageSize> buffer{};
    const int bytes_written =
        std::snprintf(buffer.data(), buffer.size(), "Check failed at %s:%u %s\nError message: %s\n",
                      file_name, line, function_name, message);
    if (unlikely(bytes_written < 0)) {
        std::perror("std::snprintf");
#if defined(__cpp_lib_to_array) && __cpp_lib_to_array >= 201907L
        constexpr std::array msg =
            std::to_array("std::snprintf failed while filling exception message");
#else
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
        constexpr char msg[] = "std::snprintf failed while filling exception message";
#endif
        static_assert(std::size(msg) < kMaxErrorMessageSize, "impl error");
        std::char_traits<char>::copy(buffer.data(), std::data(msg), std::size(msg));
        buffer[std::size(msg)] = '\0';
    }

    throw std::runtime_error(buffer.data());
}

inline void log_location_impl(const char* const file_name,
                              const std::uint32_t line,
                              const char* const function_name) noexcept {
    std::printf("%s:%u: %s\n", file_name, line, function_name);
}

inline void log_message_impl(const char* const file_name,
                             const std::uint32_t line,
                             const char* const function_name,
                             const char* const message) noexcept {
    std::printf("%s:%u: %s:\n    %s\n", file_name, line, function_name, message);
}

inline void log_message_impl(const char* const file_name,
                             const std::uint32_t line,
                             const char* const function_name,
                             const std::string_view message) noexcept {
    const auto message_size = static_cast<int>(message.size());
    std::printf("%s:%u: %s:\n    %.*s\n", file_name, line, function_name, message_size,
                message.data());
}

}  // namespace test_tools_detail

#if defined(TEST_TOOLS_HAS_SOURCE_LOCATION)

ATTRIBUTE_ALWAYS_INLINE inline void log_tests_started(
    const std::source_location src = std::source_location::current()) noexcept {
    int ret = std::printf("Started tests in %s\n", src.function_name());
    assert(ret > 0);
    ret = std::fflush(stdout);
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

ATTRIBUTE_ALWAYS_INLINE
inline void log_tests_started_impl(const char* const function_name) noexcept {
    printf("Started tests in %s\n", function_name);
}

}  // namespace test_tools_detail

#define log_tests_started() test_tools_detail::log_tests_started_impl(CONFIG_CURRENT_FUNCTION_NAME)

#define log_location() \
    test_tools_detail::log_location_impl(__FILE__, __LINE__, CONFIG_CURRENT_FUNCTION_NAME)

#define log_message(message) \
    test_tools_detail::log_message_impl(__FILE__, __LINE__, CONFIG_CURRENT_FUNCTION_NAME, message);

#endif

struct FilePtr final {
    using FileHandle = std::FILE*;
    FileHandle const file;  // NOLINT(misc-misplaced-const)

    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL /* implicit */ constexpr operator FileHandle() const noexcept {
        return file;
    }

    FilePtr(const char* const fname, const char* const mode) : file(DoFOpenOrThrow(fname, mode)) {}
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
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("impl error")
    ATTRIBUTE_RETURNS_NONNULL
    ATTRIBUTE_ALWAYS_INLINE
    static FileHandle DoFOpenOrThrow(const char* const fname, const char* const mode) {
        // NOLINTNEXTLINE(misc-misplaced-const, cppcoreguidelines-owning-memory)
        FileHandle const file_handle = std::fopen(fname, mode);
        if (unlikely(file_handle == nullptr)) {
            FilePtr::ThrowOnFOpenFail(fname, mode);
        }

        return file_handle;
    }
    [[noreturn]] ATTRIBUTE_COLD static void ThrowOnFOpenFail(const char* const fname,
                                                             const char* const mode) {
        constexpr std::size_t kMaxErrorMessageSize = 1024 * sizeof(char);
        const auto errno_value                     = errno;
        std::array<char, kMaxErrorMessageSize> buffer{};
        const int bytes_written = std::snprintf(
            buffer.data(), buffer.size(),
            "FilePtr::FilePtr(const char* fname, const char* mode): "
            "std::fopen(\"%s\", \"%s\") failed: %s",
            fname, mode, std::strerror(errno_value));  // NOLINT(concurrency-mt-unsafe)
        if (unlikely(bytes_written <= 0)) {
#if defined(__cpp_lib_to_array) && __cpp_lib_to_array >= 201907L
            constexpr std::array msg = std::to_array(
                "FilePtr::FilePtr(const char* fname, const char* mode): std::snprintf failed after "
                "std::fopen failed");
#else
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
            constexpr char msg[] =
                "FilePtr::FilePtr(const char* fname, const char* mode): std::snprintf failed after "
                "std::fopen failed";
#endif
            static_assert(std::size(msg) < kMaxErrorMessageSize, "impl error");
            std::char_traits<char>::copy(buffer.data(), std::data(msg), std::size(msg));
            buffer[std::size(msg)] = '\0';
        }

        throw std::runtime_error(buffer.data());
    }
};

template <class Observed = void>
struct EchoLogger {
    // NOLINTBEGIN(cert-oop54-cpp)

    EchoLogger() {
        ::test_tools::log_location();
    }
    EchoLogger(const EchoLogger& /*unused*/) noexcept {
        ::test_tools::log_location();
    }
    EchoLogger(EchoLogger&& /*unused*/) noexcept {
        ::test_tools::log_location();
    }
    EchoLogger& operator=(const EchoLogger& /*unused*/) noexcept {
        ::test_tools::log_location();
        return *this;
    }
    EchoLogger& operator=(EchoLogger&& /*unused*/) noexcept {
        ::test_tools::log_location();
        return *this;
    }
    ~EchoLogger() {
        ::test_tools::log_location();
    }

    // NOLINTEND(cert-oop54-cpp)
};

}  // namespace test_tools

// NOLINTEND(cppcoreguidelines-macro-usage)

#if defined(TEST_TOOLS_HAS_SOURCE_LOCATION)
#undef TEST_TOOLS_HAS_SOURCE_LOCATION
#endif
