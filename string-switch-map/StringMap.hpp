#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "config_macros.hpp"

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L && CONFIG_HAS_INCLUDE(<span>)
#define STRING_MAP_HAS_SPAN 1
#include <span>
#else
#define STRING_MAP_HAS_SPAN 0
#endif

#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L && CONFIG_HAS_INCLUDE(<bit>)
#define STRING_MAP_HAS_BIT 1
#include <bit>
#else
#define STRING_MAP_HAS_BIT 0
#endif

#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
#define STRING_MAP_CONSTEVAL consteval
#else
#define STRING_MAP_CONSTEVAL constexpr
#endif

namespace string_map_detail {

inline constexpr std::size_t kMaxStringViewSize = 200;

template <std::size_t N = kMaxStringViewSize>
struct [[nodiscard]] CompileTimeStringLiteral {
    static_assert(N > 0);
    STRING_MAP_CONSTEVAL CompileTimeStringLiteral(std::string_view str) noexcept
        : length(str.size()) {
        const bool fits_in_buffer = str.size() < std::size(value);
        // HINT: Change kMaxStringViewSize if you are using very long strings
        //  in the StringMatch / StringMap.
        [[maybe_unused]] const auto string_view_size_check = 0 / int{fits_in_buffer};
        std::char_traits<char>::copy(value.data(), str.data(), str.size());
    }
    STRING_MAP_CONSTEVAL CompileTimeStringLiteral(const char (&str)[N]) noexcept
        : length(std::char_traits<char>::length(str)) {
        std::char_traits<char>::copy(value.data(), str, size());
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL std::size_t size() const noexcept {
        return length;
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL char operator[](std::size_t index) const noexcept {
        return value[index];
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL char& operator[](std::size_t index) noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return value[index];
    }
    [[nodiscard]] STRING_MAP_CONSTEVAL std::string_view as_string_view() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::string_view(value.data(), size());
    }

    // clang-format off
    template <class CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    friend constexpr bool operator==(const CompileTimeStringLiteral<N>& str1, const std::basic_string_view<CharType> str2) noexcept {
        // clang-format on
        static_assert(std::is_same_v<CharType, char> || std::is_same_v<CharType, unsigned char>);
        if constexpr (std::is_same_v<CharType, char>) {
            return str1.as_string_view() == str2;
        } else if (std::is_constant_evaluated()) {
            const auto uvalue = std::bit_cast<std::array<unsigned char, N>>(str1.value);
            return std::basic_string_view<unsigned char>(uvalue.data(), str1.length) == str2;
        } else {
            const std::string_view cstr2(reinterpret_cast<const char*>(str2.data()), str2.size());
            return str1.as_string_view() == cstr2;
        }
    }  // clang-format off
    template <class CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    friend constexpr bool operator==(const std::basic_string_view<CharType> str1, const CompileTimeStringLiteral<N>& str2) noexcept {
        // clang-format on
        return str2 == str1;
    }

    std::array<char, N> value{};
    const std::size_t length{};
};

namespace trie_tools {

struct TrieParamsType final {
    static constexpr std::uint32_t kRootNodeIndex = 0;
    std::uint32_t min_char{};
    std::uint32_t max_char{};
    std::size_t trie_alphabet_size = max_char - min_char + 1;
    std::size_t nodes_size{};
    std::size_t max_tree_height{};

    [[nodiscard]] constexpr std::size_t CharToNodeIndex(unsigned char chr) const noexcept {
        return std::size_t{chr} - min_char;
    }
    [[nodiscard]] constexpr std::size_t CharToNodeIndex(signed char chr) const noexcept {
        return CharToNodeIndex(static_cast<unsigned char>(chr));
    }
    [[nodiscard]] constexpr std::size_t CharToNodeIndex(char chr) const noexcept {
        return CharToNodeIndex(static_cast<unsigned char>(chr));
    }
};

struct MinMaxCharsType {
    std::uint32_t min_char;
    std::uint32_t max_char;
};

template <string_map_detail::CompileTimeStringLiteral FirstString,
          string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL MinMaxCharsType FindMinMaxChars() noexcept {
    static_assert(FirstString.size() > 0, "Empty string was passed in StringMatch / StringMap");

    std::uint32_t min_char    = FirstString[0];
    std::uint32_t max_char    = FirstString[0];
    constexpr std::size_t len = FirstString.size();
    for (std::size_t i = 1; i < len; i++) {
        const std::uint32_t chr = static_cast<std::uint8_t>(FirstString[i]);
        min_char                = std::min(min_char, chr);
        max_char                = std::max(max_char, chr);
    }
    if constexpr (sizeof...(Strings) > 0) {
        constexpr MinMaxCharsType mn_mx_chars = FindMinMaxChars<Strings...>();
        min_char                              = std::min(min_char, mn_mx_chars.min_char);
        max_char                              = std::max(max_char, mn_mx_chars.max_char);
    }

    return {
        .min_char = min_char,
        .max_char = max_char,
    };
}

template <std::size_t AlphabetSize>
struct CountingVector final {
    struct CountingNode final {
        std::array<std::uint32_t, AlphabetSize> edges{};
    };

    using value_type = CountingNode;

    struct CountingVectorAllocatorImpl final {
        [[nodiscard]] constexpr value_type* allocate(std::size_t size) const {
            // Can't call ::operator new(...) in the constexpr context
            return new value_type[size]();
        }
        constexpr void deallocate(value_type* ptr,
                                  [[maybe_unused]] std::size_t size) const noexcept {
            delete[] ptr;
        }
    };
    using allocator_type = CountingVectorAllocatorImpl;

    explicit constexpr CountingVector(std::size_t size)
        : size_(size), capacity_(size_ > 16 ? size_ : 16) {
        data_ = allocator_type{}.allocate(capacity_);
    }
    constexpr CountingVector(const CountingVector& other)
        : size_(other.size_), capacity_(other.capacity_) {
        data_ = allocator_type{}.allocate(capacity_);
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
        std::copy_n(other.data_, size_, data_);
#else
        const auto other_iter = other.data_;
        for (auto iter = data_, end = iter + size_; iter != end; ++iter, ++other_iter) {
            *iter = *other_iter;
        }
#endif
    }
    constexpr void swap(CountingVector& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    friend constexpr void swap(CountingVector& lhs, CountingVector& rhs) noexcept {
        lhs.swap(rhs);
    }
    constexpr CountingVector(CountingVector&& other) noexcept
        : data_(std::exchange(other.data_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          capacity_(std::exchange(other.capacity_, 0)) {}
    constexpr CountingVector& operator=(const CountingVector& other) ATTRIBUTE_LIFETIME_BOUND {
        return *this = CountingVector(other);
    }
    constexpr CountingVector& operator=(CountingVector&& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->swap(other);
        return *this;
    }
    constexpr ~CountingVector() {
        allocator_type{}.deallocate(data_, capacity_);
        data_ = nullptr;
    }
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return size_;
    }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept {
        return capacity_;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size_ == 0;
    }
    [[nodiscard]] constexpr value_type& operator[](std::size_t index) noexcept {
        return data_[index];
    }
    [[nodiscard]] constexpr const value_type& operator[](std::size_t index) const noexcept {
        return data_[index];
    }
    constexpr void emplace_back_empty_node() {
        if (size_ == capacity_) {
            growth_storage();
        }
        ++size_;
    }
    constexpr void growth_storage() {
        const auto new_capacity = capacity_ > 0 ? capacity_ * 2 : 16;
        auto new_data           = allocator_type{}.allocate(new_capacity);
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L
        std::move(data_, data_ + size_, new_data);
#else
        auto new_data_iter = new_data;
        for (auto iter = data_, end = iter + size_; iter != end; ++iter, ++new_data_iter) {
            *new_data_iter = std::move(*iter);
        }
#endif
        allocator_type{}.deallocate(data_, capacity_);
        data_     = new_data;
        capacity_ = new_capacity;
    }

    value_type* data_{};
    std::size_t size_{};
    std::size_t capacity_{};
};

template <trie_tools::TrieParamsType TrieParams,
          string_map_detail::CompileTimeStringLiteral FirstString,
          string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL std::pair<std::size_t, std::size_t> CountNodesSizeAndMaxHeightImpl(
    CountingVector<TrieParams.trie_alphabet_size>& nodes, std::size_t max_seen_height) {
    std::size_t current_node_index = 0;
    constexpr std::size_t len      = FirstString.size();
    for (std::size_t i = 0; i < len; i++) {
        std::size_t index           = TrieParams.CharToNodeIndex(FirstString[i]);
        std::size_t next_node_index = nodes[current_node_index].edges[index];
        if (next_node_index == 0) {
            std::size_t new_node_index = nodes.size();
            nodes.emplace_back_empty_node();
            nodes[current_node_index].edges[index] = static_cast<std::uint32_t>(new_node_index);
            next_node_index                        = new_node_index;
        }
        current_node_index = next_node_index;
    }

    max_seen_height = std::max(max_seen_height, len);

    if constexpr (sizeof...(Strings) > 0) {
        return CountNodesSizeAndMaxHeightImpl<TrieParams, Strings...>(nodes, max_seen_height);
    } else {
        return {nodes.size(), max_seen_height};
    }
}

template <TrieParamsType TrieParams, string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL std::pair<std::size_t, std::size_t> CountNodesSizeAndMaxHeight() {
    constexpr auto kAlphabetSize = TrieParams.trie_alphabet_size;
    CountingVector<kAlphabetSize> nodes(std::size_t{1});
    std::size_t max_seen_height = 0;
    return CountNodesSizeAndMaxHeightImpl<TrieParams, Strings...>(nodes, max_seen_height);
}

template <string_map_detail::CompileTimeStringLiteral... Strings>
STRING_MAP_CONSTEVAL TrieParamsType TrieParams() {
    constexpr MinMaxCharsType kMinMaxChars    = FindMinMaxChars<Strings...>();
    constexpr TrieParamsType kTrieParamsProto = {
        .min_char = kMinMaxChars.min_char,
        .max_char = kMinMaxChars.max_char,
    };
    const auto [nodes_size, max_tree_height] =
        CountNodesSizeAndMaxHeight<kTrieParamsProto, Strings...>();
    return {
        .min_char        = kTrieParamsProto.min_char,
        .max_char        = kTrieParamsProto.max_char,
        .nodes_size      = nodes_size,
        .max_tree_height = max_tree_height,
    };
}

template <string_map_detail::CompileTimeStringLiteral... Strings>
inline constexpr TrieParamsType kTrieParams = TrieParams<Strings...>();

}  // namespace trie_tools

namespace string_map_impl {

template <trie_tools::TrieParamsType TrieParams, std::array MappedValues,
          typename decltype(MappedValues)::value_type DefaultMapValue,
          CompileTimeStringLiteral... Strings>
class [[nodiscard]] StringMapImplManyStrings final {
    static_assert(0 < TrieParams.min_char && TrieParams.min_char <= TrieParams.max_char &&
                      TrieParams.max_char <= std::numeric_limits<std::uint8_t>::max(),
                  "Empty string was passed in StringMatch / StringMap");
    static_assert(sizeof...(Strings) == std::size(MappedValues) && std::size(MappedValues) > 0,
                  "internal error");

    template <bool InCompileTime, bool ForceUnsignedChar = false>
    struct [[nodiscard]] InternalIterator final {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type = std::conditional_t<InCompileTime && !ForceUnsignedChar, const char,
                                              const unsigned char>;
        using pointer    = value_type*;
        using reference  = value_type&;

        template <class CharType>
        ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ALWAYS_INLINE explicit constexpr InternalIterator(
            const CharType* ATTRIBUTE_LIFETIME_BOUND str) noexcept {
            if constexpr (InCompileTime || std::is_same_v<CharType, value_type>) {
                pointer_ = str;
            } else {
#if STRING_MAP_HAS_BIT
                pointer_ = std::bit_cast<pointer>(str);
#else
                pointer_ = reinterpret_cast<pointer>(str);
#endif
            }
        }
        ATTRIBUTE_ALWAYS_INLINE constexpr InternalIterator& operator++() noexcept
            ATTRIBUTE_LIFETIME_BOUND {
            ++pointer_;
            return *this;
        }
        [[nodiscard]]
        ATTRIBUTE_PURE ATTRIBUTE_ALWAYS_INLINE constexpr value_type operator*() const noexcept {
            return *pointer_;
        }
        [[nodiscard]]
        ATTRIBUTE_PURE ATTRIBUTE_ALWAYS_INLINE constexpr value_type operator[](
            std::size_t index) const noexcept {
            return pointer_[index];
        }
        [[nodiscard]]
        ATTRIBUTE_PURE ATTRIBUTE_ALWAYS_INLINE constexpr bool operator==(
            const InternalIterator& other) const noexcept {
            return pointer_ == other.pointer_;
        }
        struct CStringSentinel {};
        [[nodiscard]]
        ATTRIBUTE_PURE ATTRIBUTE_ALWAYS_INLINE constexpr bool operator==(
            const CStringSentinel&) const noexcept {
            return *pointer_ == '\0';
        }

        pointer pointer_{};
    };

public:
    using MappedType = typename decltype(MappedValues)::value_type;
    static_assert(std::is_copy_assignable_v<MappedType>);

    static constexpr MappedType kDefaultValue = DefaultMapValue;
    static constexpr char kMinChar            = static_cast<char>(TrieParams.min_char);
    static constexpr char kMaxChar            = static_cast<char>(TrieParams.max_char);

    STRING_MAP_CONSTEVAL StringMapImplManyStrings() noexcept {
        AddPattern<0, Strings...>(kRootNodeIndex + 1);
    }

    constexpr MappedType operator()(std::nullptr_t) const noexcept              = delete;
    constexpr MappedType operator()(std::nullptr_t, std::size_t) const noexcept = delete;

    // clang-format off
    template <std::integral CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr MappedType operator()(std::basic_string_view<CharType> str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
    // clang-format off
    template <std::integral CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr MappedType operator()(const std::basic_string<CharType>& str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
    // clang-format off
    template <std::integral CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    constexpr MappedType operator()(const CharType* str, std::size_t size) const noexcept {
        // clang-format on
        if (std::is_constant_evaluated()) {
            if constexpr (std::is_same_v<CharType, char>) {
                using IteratorType = InternalIterator</*InCompileTime = */ true>;
                return operator_call_impl(IteratorType{str}, IteratorType{str + size});
            } else {
                using IteratorType =
                    InternalIterator</*InCompileTime = */ true, /*ForceUnsignedChar = */ true>;
                return operator_call_impl(IteratorType{str}, IteratorType{str + size});
            }
        } else {
            if constexpr (std::is_same_v<CharType, char>) {
                using IteratorType = InternalIterator</*InCompileTime = */ false>;
                return operator_call_impl(IteratorType{str}, IteratorType{str + size});
            } else {
                using IteratorType =
                    InternalIterator</*InCompileTime = */ false, /*ForceUnsignedChar = */ true>;
                return operator_call_impl(IteratorType{str}, IteratorType{str + size});
            }
        }
    }
    // clang-format off
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_ACCESS(read_only, 2)
    constexpr MappedType operator()(const char* str) const noexcept {
        // clang-format on
        if (str == nullptr) [[unlikely]] {
            return kDefaultValue;
        }
        if (std::is_constant_evaluated()) {
            using IteratorType = InternalIterator</*InCompileTime = */ true>;
            using SentinelType = typename IteratorType::CStringSentinel;
            return operator_call_impl(IteratorType{str}, SentinelType{});
        } else {
            using IteratorType = InternalIterator</*InCompileTime = */ false>;
            using SentinelType = typename IteratorType::CStringSentinel;
            return operator_call_impl(IteratorType{str}, SentinelType{});
        }
    }

#if STRING_MAP_HAS_SPAN
    // clang-format off
    template <std::integral CharType, std::size_t SpanExtent>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr MappedType operator()(std::span<const CharType, SpanExtent> str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
#endif

private:
    using NodeIndex = std::uint32_t;

    static constexpr NodeIndex kRootNodeIndex      = TrieParams.kRootNodeIndex;
    static constexpr std::size_t kTrieAlphabetSize = TrieParams.trie_alphabet_size;
    static constexpr std::size_t kNodesSize        = TrieParams.nodes_size;

    struct TrieNodeImpl final {
        std::array<NodeIndex, kTrieAlphabetSize> edges{};
        MappedType node_value = kDefaultValue;
    };
    std::array<TrieNodeImpl, kNodesSize> nodes_
#if !(defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 13 && __GNUC_MINOR__ == 1)
        // `internal compiler error: Segmentation fault` on gcc 13.1, see
        // https://godbolt.org/z/6EWrd8sGG
        {}
#endif
    ;

    template <std::size_t CurrentPackIndex, string_map_detail::CompileTimeStringLiteral String,
              string_map_detail::CompileTimeStringLiteral... AddStrings>
    STRING_MAP_CONSTEVAL void AddPattern(std::size_t first_free_node_index) noexcept {
        std::size_t current_node_index = 0;
        constexpr std::size_t len      = String.size();
        for (std::size_t i = 0; i < len; i++) {
            std::size_t symbol_index    = TrieParams.CharToNodeIndex(String[i]);
            std::size_t next_node_index = nodes_[current_node_index].edges[symbol_index];
            if (next_node_index == 0) {
                nodes_[current_node_index].edges[symbol_index] =
                    static_cast<NodeIndex>(first_free_node_index);
                next_node_index = first_free_node_index;
                first_free_node_index++;
            }
            current_node_index = next_node_index;
        }

        const bool already_added_string = nodes_[current_node_index].node_value != kDefaultValue;
        // HINT: Remove duplicate strings from the StringMatch / StringMap
        [[maybe_unused]] const auto duplicate_strings_check = 0 / !already_added_string;

        static_assert(CurrentPackIndex < MappedValues.size(), "impl error");
        nodes_[current_node_index].node_value = MappedValues[CurrentPackIndex];
        if constexpr (sizeof...(AddStrings) >= 1) {
            AddPattern<CurrentPackIndex + 1, AddStrings...>(first_free_node_index);
        }
    }

    // clang-format off
    template <class IteratorType, class SentinelIteratorType>
    ATTRIBUTE_PURE
    constexpr MappedType operator_call_impl(IteratorType begin, SentinelIteratorType end) const noexcept {
        // clang-format on

        std::size_t current_node_index = kRootNodeIndex;
        for (std::size_t height = 0; begin != end; ++begin, ++height) {
            std::size_t index = TrieParams.CharToNodeIndex(*begin);
            if (index >= kTrieAlphabetSize) {
                return kDefaultValue;
            }

            std::size_t next_node_index = nodes_[current_node_index].edges[index];
            if (next_node_index > 0) {
                current_node_index = next_node_index;
            } else {
                return kDefaultValue;
            }

            if (height > TrieParams.max_tree_height) {
                CONFIG_UNREACHABLE();
            }
        }

        const auto returned_value = nodes_[current_node_index].node_value;
        if constexpr (kMappedTypesInfo.ordered) {
            if (returned_value != kDefaultValue && (returned_value < kMappedTypesInfo.min_value ||
                                                    returned_value > kMappedTypesInfo.max_value)) {
                CONFIG_UNREACHABLE();
            }
        }

        return returned_value;
    }

    struct TMappedTypesInfo final {
        static constexpr bool kMaybeOrdered =
            std::is_arithmetic_v<MappedType> || std::is_pointer_v<MappedType> ||
            std::is_member_pointer_v<MappedType> ||
            (std::is_enum_v<MappedType> && requires(MappedType x, MappedType y) {
                { std::less<MappedType>{}(x, y) } -> std::same_as<bool>;
                { std::less_equal<MappedType>{}(x, y) } -> std::same_as<bool>;
                { std::greater<MappedType>{}(x, y) } -> std::same_as<bool>;
                { std::greater_equal<MappedType>{}(x, y) } -> std::same_as<bool>;
            });
        using DefaultConstructibleSentinelType =
            std::conditional_t<std::is_default_constructible_v<MappedType>, MappedType, int>;

        bool ordered = false;
        DefaultConstructibleSentinelType max_value{};
        DefaultConstructibleSentinelType min_value{};
    };

    STRING_MAP_CONSTEVAL static TMappedTypesInfo get_mapped_values_info() noexcept {
        TMappedTypesInfo info{};
        if constexpr (TMappedTypesInfo::kMaybeOrdered) {
            if constexpr (std::is_floating_point_v<MappedType>) {
                auto bad_float = [](MappedType x) constexpr noexcept -> bool {
                    return x != x || x >= std::numeric_limits<MappedType>::infinity() ||
                           x <= -std::numeric_limits<MappedType>::infinity() ||
                           x == std::numeric_limits<MappedType>::quiet_NaN() ||
                           x == std::numeric_limits<MappedType>::signaling_NaN();
                };
#if defined(__cpp_lib_constexpr_algorithms) && __cpp_lib_constexpr_algorithms >= 201806L && \
    !defined(_GLIBCXX_DEBUG) && !defined(_LIBCPP_ENABLE_ASSERTIONS)
                if (std::any_of(MappedValues.begin(), MappedValues.end(), bad_float)) {
                    return info;
                }
#else
                for (const MappedType& x : MappedValues) {
                    if (bad_float(x)) {
                        return info;
                    }
                }
#endif
            }

            info.ordered   = true;
            info.min_value = MappedValues.front();
            info.max_value = MappedValues.front();
            for (const MappedType& value : MappedValues) {
                if (std::less<MappedType>{}(value, info.min_value)) {
                    info.min_value = value;
                } else if (std::greater<MappedType>{}(value, info.max_value)) {
                    info.max_value = value;
                }
            }
        }

        return info;
    }

    static constexpr TMappedTypesInfo kMappedTypesInfo = get_mapped_values_info();
};

template <trie_tools::TrieParamsType TrieParams, std::array MappedValues,
          typename decltype(MappedValues)::value_type DefaultMapValue,
          CompileTimeStringLiteral... Strings>
class [[nodiscard]] StringMapImplFewStrings final {
    static_assert(0 < TrieParams.min_char && TrieParams.min_char <= TrieParams.max_char &&
                      TrieParams.max_char <= std::numeric_limits<std::uint8_t>::max(),
                  "Empty string was passed in StringMatch / StringMap");
    static_assert(sizeof...(Strings) == std::size(MappedValues) && std::size(MappedValues) > 0,
                  "internal error");

public:
    using MappedType = typename decltype(MappedValues)::value_type;
    static_assert(std::is_copy_assignable_v<MappedType>);

    static constexpr MappedType kDefaultValue = DefaultMapValue;
    static constexpr char kMinChar            = static_cast<char>(TrieParams.min_char);
    static constexpr char kMaxChar            = static_cast<char>(TrieParams.max_char);

    STRING_MAP_CONSTEVAL StringMapImplFewStrings() noexcept = default;

    constexpr MappedType operator()(std::nullptr_t) const noexcept              = delete;
    constexpr MappedType operator()(std::nullptr_t, std::size_t) const noexcept = delete;

    // clang-format off
    template <class CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr MappedType operator()(std::basic_string_view<CharType> str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
    // clang-format off
    template <class CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE    
    constexpr MappedType operator()(const std::basic_string<CharType>& str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
    // clang-format off
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_ACCESS(read_only, 2)
    constexpr MappedType operator()(const char* str) const noexcept {
        // clang-format on
        if (str == nullptr) [[unlikely]] {
            return kDefaultValue;
        }
        return operator()(str, std::char_traits<char>::length(str));
    }
    // clang-format off
    template <class CharType>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    constexpr MappedType operator()(const CharType* str, std::size_t size) const noexcept {
        // clang-format on
        return operator_call_impl<CharType, 0, Strings...>(str, size);
    }

#if STRING_MAP_HAS_SPAN
    // clang-format off
    template <class CharType, std::size_t SpanExtent>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr MappedType operator()(std::span<const CharType, SpanExtent> str) const noexcept {
        // clang-format on
        return operator()(str.data(), str.size());
    }
#endif

private:
    // clang-format off
    template <class CharType, std::size_t Index, CompileTimeStringLiteral CompString, CompileTimeStringLiteral... CompStrings>
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
    static constexpr MappedType operator_call_impl(const CharType* str, std::size_t size) noexcept {
        // clang-format on
        if (CompString == std::basic_string_view<CharType>(str, size)) {
            static_assert(Index < std::size(MappedValues));
            return MappedValues[Index];
        }
        if constexpr (sizeof...(CompStrings) >= 1) {
            return operator_call_impl<CharType, Index + 1, CompStrings...>(str, size);
        }
        return kDefaultValue;
    }
};

}  // namespace string_map_impl

}  // namespace string_map_detail

template <string_map_detail::CompileTimeStringLiteral... Strings>
struct StringKeys;

template <class T, std::size_t N>
struct MapValues {
    std::array<T, N> values;
};

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201606
template <typename T, typename... Ts>
MapValues(T, Ts...)
    -> MapValues<std::enable_if_t<(std::is_same_v<T, Ts> && ...), T>, 1 + sizeof...(Ts)>;
#endif

namespace string_map_detail {

template <class Keys, MapValues MappedValues, auto MappedDefaultValue>
struct StringHelper;

template <string_map_detail::CompileTimeStringLiteral... Strings, MapValues MappedValues,
          class MappedType, MappedType MappedDefaultValue>
    requires(sizeof...(Strings) == std::size(MappedValues.values) &&
             std::size(MappedValues.values) > 0) &&
            std::is_same_v<typename decltype(MappedValues.values)::value_type, MappedType>
struct StringHelper<StringKeys<Strings...>, MappedValues, MappedDefaultValue> {
    using type = typename std::conditional_t<
        (sizeof...(Strings) <= 4 &&
         string_map_detail::trie_tools::kTrieParams<Strings...>.max_tree_height <= 15),
        typename string_map_detail::string_map_impl::StringMapImplFewStrings<
            string_map_detail::trie_tools::kTrieParams<Strings...>, MappedValues.values,
            MappedDefaultValue, Strings...>,
        typename string_map_detail::string_map_impl::StringMapImplManyStrings<
            string_map_detail::trie_tools::kTrieParams<Strings...>, MappedValues.values,
            MappedDefaultValue, Strings...>>;
};

template <std::size_t N>
STRING_MAP_CONSTEVAL MapValues<std::size_t, N> make_index_array_for_map() noexcept {
    MapValues<std::size_t, N> index_array{};
#if defined(__cpp_lib_constexpr_numeric) && __cpp_lib_constexpr_numeric >= 201911L && \
    !defined(_GLIBCXX_DEBUG) && !defined(_LIBCPP_ENABLE_ASSERTIONS)
    std::iota(index_array.values.begin(), index_array.values.end(), 0);
#else
    for (std::size_t i = 0; i < index_array.values.size(); i++) {
        index_array.values[i] = i;
    }
#endif
    return index_array;
}

}  // namespace string_map_detail

#undef STRING_MAP_CONSTEVAL
#undef STRING_MAP_HAS_BIT
#undef STRING_MAP_HAS_SPAN

template <class Keys, MapValues Values, auto DefaultValue>
using StringMap = typename string_map_detail::StringHelper<Keys, Values, DefaultValue>::type;

template <string_map_detail::CompileTimeStringLiteral... Strings>
using StringMatch = StringMap<StringKeys<Strings...>,
                              string_map_detail::make_index_array_for_map<sizeof...(Strings)>(),
                              sizeof...(Strings)>;
