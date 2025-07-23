// clang-format off
#include "../get_typename.hpp"
// clang-format on

#include <cassert>
#include <string>

// clang-format off
#include "../get_typename.hpp"
// clang-format on

enum class E {
    kValue1 = 1,
    kValue2,
    kValue3,
};

namespace ns1 {

class C {};

struct S {};

namespace ns2 {
struct C1 {
    struct C2 {
        struct C3 {};
    };
};
enum E1 : bool { kE };
}  // namespace ns2

enum E2 : bool { kE };

}  // namespace ns1

static_assert(misc::get_qualified_typename<E>() == "E");
static_assert(misc::get_qualified_typename<ns1::C>() == "ns1::C");
static_assert(misc::get_qualified_typename<ns1::S>() == "ns1::S");
static_assert(misc::get_qualified_typename<ns1::ns2::C1::C2::C3>() == "ns1::ns2::C1::C2::C3");
static_assert(misc::get_enum_value_name<E::kValue1>() == "kValue1");
static_assert(misc::get_enum_value_name<ns1::ns2::E1::kE>() == "kE");
static_assert(misc::get_enum_value_name<ns1::E2::kE>() == "kE");

int main() {
    assert(misc::get_qualified_typename<E>() == "E");
    assert(misc::get_qualified_typename<ns1::C>() == "ns1::C");
    assert(misc::get_qualified_typename<ns1::S>() == "ns1::S");
    assert(misc::get_qualified_typename<ns1::ns2::C1::C2::C3>() == "ns1::ns2::C1::C2::C3");
    assert(std::string{misc::get_qualified_typename<ns1::ns2::C1::C2::C3>()} ==
           "ns1::ns2::C1::C2::C3");
    assert(misc::get_enum_value_name<E::kValue1>() == "kValue1");
    assert(misc::get_enum_value_name<ns1::ns2::E1::kE>() == "kE");
    assert(misc::get_enum_value_name<ns1::E2::kE>() == "kE");

    return 0;
}
