#ifndef STRING_TRAITS_INCLUDING_IMPLEMENTATION
// cppcheck-suppress [preprocessorErrorDirective]
#error This header should not be used directly
#endif

#include <string>
#include <string_view>
#include <type_traits>

namespace misc::string_detail {

template <class T, class CharType>
struct same_char_types
    : std::conditional_t<misc::is_char_v<T>, std::is_same<T, CharType>, std::true_type> {};

template <class StrCharType, class CharType>
struct same_char_types<std::basic_string<StrCharType>, CharType>
    : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType>
struct same_char_types<std::basic_string_view<StrCharType>, CharType>
    : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType, size_t N>
struct same_char_types<StrCharType[N], CharType>
    : std::is_same<std::remove_cv_t<StrCharType>, CharType> {};

template <class StrCharType, class CharType>
struct same_char_types<StrCharType *, CharType>
    : std::conditional_t<misc::is_char_v<std::remove_cv_t<StrCharType>>,
                         std::is_same<std::remove_cv_t<StrCharType>, CharType>,
                         std::true_type> {};

struct dummy_base {};

template <class T>
struct dummy_base_with_type {
    using type = T;
};

template <class... Types>
struct determine_char_type {
    using type = void;
};

template <class... Types>
struct recursion_selector final {
    using type = typename determine_char_type<Types...>::type;
};

template <class CharType>
struct char_selector final {
    using type = CharType;
};

template <class FirstType, class... Types>
struct determine_char_type<FirstType, Types...> {
    using selector = std::
        conditional_t<is_char_v<FirstType>, char_selector<FirstType>, recursion_selector<Types...>>;

    using type = typename selector::type;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string_view<CharType>, Types...> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string<CharType>, Types...> {
    using type = CharType;
};

template <class CharType, size_t N, class... Types>
struct determine_char_type<const CharType[N], Types...>
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, size_t N, class... Types>
struct determine_char_type<CharType[N], Types...>
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<CharType *, Types...>
    : std::conditional_t<is_char_v<std::remove_cv_t<CharType>>,
                         dummy_base_with_type<std::remove_cv_t<CharType>>,
                         determine_char_type<Types...>> {
    using Base = std::conditional_t<is_char_v<std::remove_cv_t<CharType>>,
                                    dummy_base_with_type<std::remove_cv_t<CharType>>,
                                    determine_char_type<Types...>>;
    using type = typename Base::type;
};

template <class... Args>
using determine_char_t = typename determine_char_type<Args...>::type;

}  // namespace misc::string_detail
