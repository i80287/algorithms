#include <cstdint>

#include "enum_flag_utils.hpp"

namespace some {

enum CStyleEnum {
    zero  = 0,
    one   = 1 << 0,
    two   = 1 << 1,
    four  = 1 << 2,
    eight = 1 << 3,
};

GENERATE_ENUM_FLAG_BIT_OPERATIONS(CStyleEnum)

enum struct EnumClass1 : uint32_t {
    zero  = 0,
    one   = 1 << 0,
    two   = 1 << 1,
    four  = 1 << 2,
    eight = 1 << 3,
};

GENERATE_ENUM_FLAG_BIT_OPERATIONS(EnumClass1)

}  // namespace some

enum class EnumClass2 {
    zero  = 0,
    one   = 1 << 0,
    two   = 1 << 1,
    four  = 1 << 2,
    eight = 1 << 3,
};

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

ATTRIBUTE_CONST int main() {
    using some::CStyleEnum;

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(CStyleEnum);

    using some::EnumClass1;

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(EnumClass1);

    GENERATE_ENUM_FLAG_BIT_OPERATIONS_ASSERTS(EnumClass2);

    return 0;
}
