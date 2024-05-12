#include <cstdint>
#include <string>
#include <utility>
#include <cstdint>

namespace detail {

using int32_t  = std::int32_t;
using size_t   = std::size_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using uint8_t  = std::uint8_t;

inline constexpr size_t kMaxStringViewSize = 200;

template <size_t N = kMaxStringViewSize>
struct [[nodiscard]] CompileTimeStringLiteral final {
    static_assert(N > 0);

    consteval CompileTimeStringLiteral(std::string_view str) noexcept : length(str.size()) {
        // Change kMaxStringViewSize if you are using
        //  very long strings in the StringSet / StringMap.
        [[maybe_unused]] const auto string_view_size_check =
            0 / (str.size() <= std::size(value));
        std::char_traits<char>::copy(value, str.data(), str.size());
    }

    consteval CompileTimeStringLiteral(const char (&str)[N]) noexcept
        : length(str[N - 1] == '\0' ? N - 1 : N) {
        std::char_traits<char>::copy(value, str, N);
    }

    [[nodiscard]] consteval size_t size() const noexcept {
        return length;
    }

    char value[N]{};
    const size_t length{};
};

}  // namespace detail

template <class ValueType, ValueType... Values>
struct TypedValuesList {
    using Type = ValueType;
    static constexpr size_t size() noexcept { return sizeof...(Values); }
    static constexpr size_t empty() noexcept { return size() == 0; }
};

template <detail::CompileTimeStringLiteral... Strings>
struct StringsList {
    static constexpr size_t size() noexcept { return sizeof...(Strings); }
    static constexpr size_t empty() noexcept { return size() == 0; }
};

namespace detail {

template <class T>
struct IntegerSequenceToTypedValuesListImpl;

template <class T, T... Idx>
struct IntegerSequenceToTypedValuesListImpl<std::integer_sequence<T, Idx...>> {
    using Type = TypedValuesList<T, Idx...>;
};

template <class T>
struct TypedValuesListHeadImpl;

template <class ValueType, ValueType Value, ValueType... Values>
struct TypedValuesListHeadImpl<TypedValuesList<ValueType, Value, Values...>> {
    static constexpr auto kValue = Value;
};

template <class T>
struct TypedValuesListTail;

template <class ValueType, ValueType Value, ValueType... Values>
struct TypedValuesListTail<TypedValuesList<ValueType, Value, Values...>> {
    using Type = TypedValuesList<ValueType, Values...>;
};

template <std::size_t CurrentIndex, std::size_t Index, class T, T Arg, T... Args>
consteval T PackElementAtIndexImpl() {
    if constexpr (CurrentIndex == Index) {
        return Arg;
    } else {
        return PackElementAtIndexImpl<CurrentIndex + 1, Index, T, Args...>();
    }
}

}  // namespace detail

template <class T>
using IntegerSequenceToTypedValuesList = detail::IntegerSequenceToTypedValuesListImpl<T>::Type;

template <class T>
inline constexpr auto TypedValuesListHead = detail::TypedValuesListHeadImpl<T>::kValue;

template <class T>
using TypedValuesListTail = detail::TypedValuesListTail<T>::Type;

template <std::size_t Index, class T, T... Args>
    requires (Index < sizeof...(Args))
consteval T PackElementAtIndex() {
    return detail::PackElementAtIndexImpl<0, Index, T, Args...>();
}
