#pragma once

#include <string_view>

#include "config_macros.hpp"

namespace misc {

template <class T>
[[nodiscard]] constexpr std::string_view get_qualified_typename();

template <class T>
[[nodiscard]] constexpr std::string_view get_unqualified_typename();

template <auto EnumValue>
[[nodiscard]] constexpr std::string_view get_enum_value_name();

}  // namespace misc

#define GET_TYPENAME_INCLUDING_IMPLEMENTATION
#include "get_typename.ipp"
#undef GET_TYPENAME_INCLUDING_IMPLEMENTATION
