// clang-format off
#include "../enum_utils.hpp"
// clang-format on

#include <cassert>
#include <cstdint>

// clang-format off
#include "../enum_utils.hpp"
// clang-format on

#if CONFIG_HAS_AT_LEAST_CXX_17
#ifndef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS
#error GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS() should be available since C++17
#endif
#endif

namespace some {

enum CStyleEnum {
    zero = 0,
    one = 1 << 0,
    two = 1 << 1,
    four = 1 << 2,
    eight = 1 << 3,
};

GENERATE_ENUM_FLAG_BIT_OPERATIONS(CStyleEnum)

GENERATE_ENUM_TO_INTEGER(CStyleEnum)

#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS

GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(CStyleEnum,
                                         CStyleEnum::zero,
                                         CStyleEnum::one,
                                         CStyleEnum::two,
                                         CStyleEnum::four,
                                         CStyleEnum::eight)

#endif

enum struct EnumClass1 : std::uint32_t {
    zero = 0,
    one = 1 << 0,
    two = 1 << 1,
    four = 1 << 2,
    eight = 1 << 3,
};

GENERATE_ENUM_FLAG_BIT_OPERATIONS(EnumClass1)

GENERATE_ENUM_TO_INTEGER(EnumClass1)

#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS

GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(EnumClass1,
                                         EnumClass1::zero,
                                         EnumClass1::one,
                                         EnumClass1::two,
                                         EnumClass1::four,
                                         EnumClass1::eight)

#endif

}  // namespace some

enum class EnumClass2 : std::int8_t {
    zero = 0,
    one = 1 << 0,
    two = 1 << 1,
    four = 1 << 2,
    eight = 1 << 3,
};

GENERATE_ENUM_TO_INTEGER(EnumClass2)

#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS

GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS(EnumClass2,
                                         EnumClass2::zero,
                                         EnumClass2::one,
                                         EnumClass2::two,
                                         EnumClass2::four,
                                         EnumClass2::eight)

#endif

GENERATE_ENUM_FLAG_BIT_OPERATIONS(EnumClass2)

#define GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(enum_type)                           \
    static_assert((enum_type::zero | enum_type::one) == enum_type::one, "");           \
    static_assert((enum_type::zero | enum_type::two) == enum_type::two, "");           \
    static_assert((enum_type::zero | enum_type::four) == enum_type::four, "");         \
    static_assert((enum_type::zero | enum_type::eight) == enum_type::eight, "");       \
                                                                                       \
    static_assert((enum_type::one | enum_type::zero) == enum_type::one, "");           \
    static_assert((enum_type::two | enum_type::zero) == enum_type::two, "");           \
    static_assert((enum_type::four | enum_type::zero) == enum_type::four, "");         \
    static_assert((enum_type::eight | enum_type::zero) == enum_type::eight, "");       \
                                                                                       \
    static_assert((enum_type::one | enum_type::one) == enum_type::one, "");            \
    static_assert((enum_type::two | enum_type::two) == enum_type::two, "");            \
    static_assert((enum_type::four | enum_type::four) == enum_type::four, "");         \
    static_assert((enum_type::eight | enum_type::eight) == enum_type::eight, "");      \
                                                                                       \
    static_assert(static_cast<int>(enum_type::one | enum_type::two | enum_type::four | \
                                   enum_type::eight) == 15,                            \
                  "");                                                                 \
                                                                                       \
    static_assert((enum_type::zero & enum_type::one) == enum_type::zero, "");          \
    static_assert((enum_type::zero & enum_type::two) == enum_type::zero, "");          \
    static_assert((enum_type::zero & enum_type::four) == enum_type::zero, "");         \
    static_assert((enum_type::zero & enum_type::eight) == enum_type::zero, "");        \
                                                                                       \
    static_assert((enum_type::one & enum_type::zero) == enum_type::zero, "");          \
    static_assert((enum_type::two & enum_type::zero) == enum_type::zero, "");          \
    static_assert((enum_type::four & enum_type::zero) == enum_type::zero, "");         \
    static_assert((enum_type::eight & enum_type::zero) == enum_type::zero, "");        \
                                                                                       \
    static_assert((enum_type::one & enum_type::one) == enum_type::one, "");            \
    static_assert((enum_type::two & enum_type::two) == enum_type::two, "");            \
    static_assert((enum_type::four & enum_type::four) == enum_type::four, "");         \
    static_assert((enum_type::eight & enum_type::eight) == enum_type::eight, "");      \
                                                                                       \
    static_assert(static_cast<int>(enum_type::one & enum_type::two & enum_type::four & \
                                   enum_type::eight) == 0,                             \
                  "");                                                                 \
                                                                                       \
    static_assert((enum_type::zero ^ enum_type::one) == enum_type::one, "");           \
    static_assert((enum_type::zero ^ enum_type::two) == enum_type::two, "");           \
    static_assert((enum_type::zero ^ enum_type::four) == enum_type::four, "");         \
    static_assert((enum_type::zero ^ enum_type::eight) == enum_type::eight, "");       \
                                                                                       \
    static_assert((enum_type::one ^ enum_type::zero) == enum_type::one, "");           \
    static_assert((enum_type::two ^ enum_type::zero) == enum_type::two, "");           \
    static_assert((enum_type::four ^ enum_type::zero) == enum_type::four, "");         \
    static_assert((enum_type::eight ^ enum_type::zero) == enum_type::eight, "");       \
                                                                                       \
    static_assert((enum_type::one ^ enum_type::one) == enum_type::zero, "");           \
    static_assert((enum_type::two ^ enum_type::two) == enum_type::zero, "");           \
    static_assert((enum_type::four ^ enum_type::four) == enum_type::zero, "");         \
    static_assert((enum_type::eight ^ enum_type::eight) == enum_type::zero, "");       \
                                                                                       \
    static_assert(static_cast<int>(enum_type::one ^ enum_type::two ^ enum_type::four ^ \
                                   enum_type::eight) == 15,                            \
                  "");                                                                 \
    static_assert(true, "")

#define GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, int_type)                                 \
    static_assert(to_integer<int_type>(enum_type::zero) == static_cast<int_type>(enum_type::zero), \
                  "");                                                                             \
    static_assert(to_integer<int_type>(enum_type::one) == static_cast<int_type>(enum_type::one),   \
                  "");                                                                             \
    static_assert(to_integer<int_type>(enum_type::two) == static_cast<int_type>(enum_type::two),   \
                  "");                                                                             \
    static_assert(to_integer<int_type>(enum_type::four) == static_cast<int_type>(enum_type::four), \
                  "");                                                                             \
    static_assert(                                                                                 \
        to_integer<int_type>(enum_type::eight) == static_cast<int_type>(enum_type::eight), "")

#define GENERATE_ENUM_TO_INTEGER_ASSERTS(enum_type)                   \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, char);           \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, signed char);    \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, unsigned char);  \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, short);          \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, unsigned short); \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, int);            \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, unsigned);       \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, long);           \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, unsigned long);  \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, long long);      \
    GENERATE_ENUM_TO_INTEGER_ASSERTS_IMPL(enum_type, unsigned long long)

#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS

#define GENERATE_ENUM_TO_STRING_ASSERTS(enum_type)              \
    static_assert(to_string_view(enum_type::zero) == "zero");   \
    static_assert(to_string_view(enum_type::one) == "one");     \
    static_assert(to_string_view(enum_type::two) == "two");     \
    static_assert(to_string_view(enum_type::four) == "four");   \
    static_assert(to_string_view(enum_type::eight) == "eight"); \
    assert(to_string(enum_type::zero) == "zero");               \
    assert(to_string(enum_type::one) == "one");                 \
    assert(to_string(enum_type::two) == "two");                 \
    assert(to_string(enum_type::four) == "four");               \
    assert(to_string(enum_type::eight) == "eight")

#endif

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=const"
#endif

int main() {
    using some::CStyleEnum;

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(CStyleEnum);
    GENERATE_ENUM_TO_INTEGER_ASSERTS(CStyleEnum);
#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS
    GENERATE_ENUM_TO_STRING_ASSERTS(CStyleEnum);
#endif

    using some::EnumClass1;

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(EnumClass1);
    GENERATE_ENUM_TO_INTEGER_ASSERTS(EnumClass1);
#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS
    GENERATE_ENUM_TO_STRING_ASSERTS(EnumClass1);
#endif

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(EnumClass2);
    GENERATE_ENUM_TO_INTEGER_ASSERTS(EnumClass2);
#ifdef GENERATE_ENUM_TO_STRING_FOR_ENUM_MEMBERS
    GENERATE_ENUM_TO_STRING_ASSERTS(EnumClass2);
#endif

    return 0;
}

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
