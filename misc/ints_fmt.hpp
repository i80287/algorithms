#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

namespace ints_fmt {

namespace detail {

using std::size_t;

inline constexpr size_t kMaxStringLengthI8 = std::char_traits<char>::length("-128");
inline constexpr size_t kMaxStringLengthU8 = std::char_traits<char>::length("255");
inline constexpr size_t kMaxStringLengthI16 = std::char_traits<char>::length("-32768");
inline constexpr size_t kMaxStringLengthU16 = std::char_traits<char>::length("65535");
inline constexpr size_t kMaxStringLengthI32 = std::char_traits<char>::length("-2147483648");
inline constexpr size_t kMaxStringLengthU32 = std::char_traits<char>::length("4294967295");
inline constexpr size_t kMaxStringLengthI64 = std::char_traits<char>::length("-9223372036854775808");
inline constexpr size_t kMaxStringLengthU64 = std::char_traits<char>::length("18446744073709551615");
inline constexpr size_t kMaxStringLengthI128 =
    std::char_traits<char>::length("-170141183460469231731687303715884105728");
inline constexpr size_t kMaxStringLengthU128 =
    std::char_traits<char>::length("340282366920938463463374607431768211455");

template <typename T>
inline constexpr bool kIsSignedInt = static_cast<T>(-1) < 0;

template <typename T>
[[nodiscard]] constexpr size_t MaxStrLength() noexcept {
    constexpr bool kIsSigned = kIsSignedInt<T>;
    if constexpr (sizeof(T) == sizeof(std::int8_t)) {
        return kIsSigned ? kMaxStringLengthI8 : kMaxStringLengthU8;
    } else if constexpr (sizeof(T) == sizeof(std::int16_t)) {
        return kIsSigned ? kMaxStringLengthI16 : kMaxStringLengthU16;
    } else if constexpr (sizeof(T) == sizeof(std::int32_t)) {
        return kIsSigned ? kMaxStringLengthI32 : kMaxStringLengthU32;
    } else if constexpr (sizeof(T) == sizeof(std::int64_t)) {
        return kIsSigned ? kMaxStringLengthI64 : kMaxStringLengthU64;
    } else if constexpr (sizeof(T) == 16) {
        return kIsSigned ? kMaxStringLengthI128 : kMaxStringLengthU128;
    } else {
        static_assert(sizeof(T) == 0, "Integral type is expected");
        return 0;
    }
}

/// @brief Realization is taken from the gcc libstdc++ __to_chars_10_impl
template <typename U>
ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_RETURNS_NONNULL [[nodiscard]]
constexpr char* format_uint_to_buffer(U number, char* buffer_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept {
    constexpr std::uint8_t remainders[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    constexpr std::uint32_t kBase1 = 10;
    constexpr std::uint32_t kBase2 = kBase1 * kBase1;

    while (number >= kBase2) {
        const auto remainder_index = static_cast<std::size_t>(number % kBase2) * 2;
        number /= kBase2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    }

    if (number >= kBase1) {
        const auto remainder_index = static_cast<size_t>(number) * 2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    } else {
        *--buffer_ptr = static_cast<char>('0' + number);
    }

    return buffer_ptr;
}

struct FillResult {
    using size_type = std::uint8_t;

    const char* data;
    size_type size;
};

}  // namespace detail

template <typename T, typename UT = std::make_unsigned_t<T>>
class Formatter {
private:
    static constexpr std::size_t kBufferCapacity = detail::MaxStrLength<T>();

    // kBufferCapacity <= std::numeric_limits<detail::FillResult::size_type>::max() without including <limits>
    static_assert(kBufferCapacity == static_cast<detail::FillResult::size_type>(kBufferCapacity), "");

    using storage_type = std::array<char, kBufferCapacity>;

    [[nodiscard]] static constexpr UT uabs(const T value) noexcept {
        if constexpr (std::is_same_v<T, UT>) {
            return value;
        } else {
            return value >= 0 ? static_cast<UT>(value) : static_cast<UT>(-static_cast<UT>(value));
        }
    }

    [[nodiscard]]
    static constexpr detail::FillResult fill_buffer(const T number,
                                                    storage_type& buffer ATTRIBUTE_LIFETIME_BOUND) noexcept {
        char* const buffer_end_ptr = buffer.data() + buffer.size();

        const UT unum = Formatter::uabs(number);
        using ExtUT = std::conditional_t<sizeof(UT) < sizeof(unsigned), unsigned, UT>;
        char* fmt_ptr = detail::format_uint_to_buffer(ExtUT{unum}, buffer_end_ptr);
        if constexpr (detail::kIsSignedInt<T>) {
            if (number < 0) {
                *--fmt_ptr = '-';
            }
        }

        const char* const buffer_begin_ptr = buffer.data();
        CONFIG_ASSUME_STATEMENT(buffer_begin_ptr <= fmt_ptr);
        CONFIG_ASSUME_STATEMENT(fmt_ptr < buffer_end_ptr);
        const auto length = static_cast<std::size_t>(buffer_end_ptr - fmt_ptr);
        const std::size_t buffer_size = buffer.size();
        CONFIG_ASSUME_STATEMENT(1 <= length);
        CONFIG_ASSUME_STATEMENT(length <= buffer_size);
        return {fmt_ptr, static_cast<detail::FillResult::size_type>(length)};
    }

public:
    explicit constexpr Formatter(const T number) noexcept : storage_(), fill_result_(fill_buffer(number, storage_)) {}

    ATTRIBUTE_PURE [[nodiscard]] static constexpr std::size_t buffer_capacity() noexcept {
        return kBufferCapacity;
    }

    ATTRIBUTE_PURE [[nodiscard]] constexpr const char* data() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        const char* const buffer_begin_ptr = storage_.data();
        const char* const buffer_end_ptr = buffer_begin_ptr + storage_.size();
        const char* const fmt_ptr = fill_result_.data;
        CONFIG_ASSUME_STATEMENT(buffer_begin_ptr <= fmt_ptr);
        CONFIG_ASSUME_STATEMENT(fmt_ptr < buffer_end_ptr);
        return fmt_ptr;
    }

    ATTRIBUTE_PURE [[nodiscard]] constexpr std::size_t size() const noexcept {
        const detail::FillResult::size_type fmt_size = fill_result_.size;
        CONFIG_ASSUME_STATEMENT(1 <= fmt_size);
        CONFIG_ASSUME_STATEMENT(fmt_size <= buffer_capacity());
        return fmt_size;
    }

    ATTRIBUTE_PURE [[nodiscard]] constexpr std::string_view as_string_view() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::string_view{data(), size()};
    }

    ATTRIBUTE_PURE
    [[nodiscard]] explicit constexpr operator std::string_view() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return as_string_view();
    }

    [[nodiscard]] std::string as_string() const {
        return std::string{as_string_view()};
    }

    [[nodiscard]] std::wstring as_wstring() const {
        return std::wstring(data(), data() + size());
    }

private:
    storage_type storage_;
    detail::FillResult fill_result_;
};

using Int8Formatter = Formatter<std::int8_t>;
using UInt8Formatter = Formatter<std::uint8_t>;
using Int16Formatter = Formatter<std::int16_t>;
using UInt16Formatter = Formatter<std::uint16_t>;
using Int32Formatter = Formatter<std::int32_t>;
using UInt32Formatter = Formatter<std::uint32_t>;
using Int32Formatter = Formatter<std::int32_t>;
using UInt32Formatter = Formatter<std::uint32_t>;
using Int64Formatter = Formatter<std::int64_t>;
using UInt64Formatter = Formatter<std::uint64_t>;

}  // namespace ints_fmt
