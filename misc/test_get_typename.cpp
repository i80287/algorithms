#include <cassert>

#include "get_typename.hpp"

enum class E {
    kValue1 = 1,
    kValue2,
    kValue3,
};

namespace ns1 {

class C {};

}  // namespace ns1

static_assert(misc::get_typename<E>() == "E");
static_assert(misc::get_typename<ns1::C>() == "ns1::C");
static_assert(misc::get_enum_value_name<E::kValue1>() == "kValue1");

int main() {
    assert(misc::get_typename<E>() == "E");
    assert(std::string(misc::get_typename<E>()) == "E");
    assert(misc::get_typename<ns1::C>() == "ns1::C");
    assert(std::string(misc::get_typename<ns1::C>()) == "ns1::C");
    assert(misc::get_enum_value_name<E::kValue1>() == "kValue1");
    assert(std::string(misc::get_enum_value_name<E::kValue1>()) == "kValue1");
    return 0;
}
