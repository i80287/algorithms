#ifdef NDEBUG
#warning "Can't test properly with NDEBUG macro defined (macro won't be undefined manually)"
#endif

#include <mpfr.h>

#include <cassert>
#include <cinttypes>
#include <limits>

#include "math_functions.hpp"
#include "test_tools.hpp"

using namespace math_functions;
using namespace test_tools;
using std::gcd;

static_assert(bin_pow_mod(uint32_t(7), uint32_t(483), uint32_t(1000000007u)) == 263145387u,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint32_t(289), uint32_t(-1), uint32_t(2146514599u)) == 1349294778u,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint32_t(2146526839u), uint32_t(578423432u), uint32_t(2147483629u)) ==
                  281853233u,
              "bin_pow_mod");

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
static_assert(bin_pow_mod(uint64_t(119999999927ull), uint64_t(18446744073709515329ull),
                          uint64_t(100000000000000003ull)) == 85847679703545452ull,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint64_t(72057594037927843ull), uint64_t(18446744073709515329ull),
                          uint64_t(1000000000000000003ull)) == 404835689235904145ull,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint64_t(999999999999999487ull), uint64_t(18446744073709551557ull),
                          uint64_t(1000000000000000009ull)) == 802735487082721113ull,
              "bin_pow_mod");
#endif

static_assert(isqrt(0u) == 0, "isqrt");
static_assert(isqrt(1u) == 1, "isqrt");
static_assert(isqrt(4u) == 2, "isqrt");
static_assert(isqrt(9u) == 3, "isqrt");
static_assert(isqrt(10u) == 3, "isqrt");
static_assert(isqrt(15u) == 3, "isqrt");
static_assert(isqrt(16u) == 4, "isqrt");
static_assert(isqrt(257u * 257u) == 257, "isqrt");
static_assert(isqrt(257u * 257u + 1) == 257, "isqrt");
static_assert(isqrt(258u * 258u - 1u) == 257, "isqrt");
static_assert(isqrt(1u << 12) == 1 << 6, "isqrt");
static_assert(isqrt(1u << 14) == 1 << 7, "isqrt");
static_assert(isqrt(1u << 16) == 1 << 8, "isqrt");
static_assert(isqrt(1u << 28) == 1 << 14, "isqrt");
static_assert(isqrt(1u << 30) == 1 << 15, "isqrt");
static_assert(isqrt(uint32_t(-1)) == (1u << 16) - 1, "isqrt");

static_assert(isqrt(uint64_t(0)) == 0, "isqrt");
static_assert(isqrt(uint64_t(1)) == 1, "isqrt");
static_assert(isqrt(uint64_t(4)) == 2, "isqrt");
static_assert(isqrt(uint64_t(9)) == 3, "isqrt");
static_assert(isqrt(uint64_t(10)) == 3, "isqrt");
static_assert(isqrt(uint64_t(15)) == 3, "isqrt");
static_assert(isqrt(uint64_t(16)) == 4, "isqrt");
static_assert(isqrt(uint64_t(257 * 257)) == 257, "isqrt");
static_assert(isqrt(uint64_t(257 * 257 + 1)) == 257, "isqrt");
static_assert(isqrt(uint64_t(258 * 258 - 1)) == 257, "isqrt");
static_assert(isqrt(uint64_t(1 << 12)) == 1 << 6, "isqrt");
static_assert(isqrt(uint64_t(1 << 14)) == 1 << 7, "isqrt");
static_assert(isqrt(uint64_t(1 << 16)) == 1 << 8, "isqrt");
static_assert(isqrt(uint64_t(1 << 28)) == 1 << 14, "isqrt");
static_assert(isqrt(uint64_t(1 << 30)) == 1 << 15, "isqrt");
static_assert(isqrt(uint64_t(1) << 54) == uint64_t(1) << 27, "isqrt");
static_assert(isqrt(uint64_t(1) << 56) == uint64_t(1) << 28, "isqrt");
static_assert(isqrt(uint64_t(1) << 58) == uint64_t(1) << 29, "isqrt");
static_assert(isqrt(uint64_t(1) << 60) == uint64_t(1) << 30, "isqrt");
static_assert(isqrt(uint64_t(1) << 62) == uint64_t(1) << 31, "isqrt");
static_assert(isqrt(uint64_t(-1)) == 0xFFFFFFFFu, "isqrt");
static_assert(isqrt(uint64_t(1000000007) * 1000000007) == 1000000007u, "isqrt");

#if defined(INTEGERS_128_BIT_HPP) && defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

static_assert(isqrt(uint128_t(0)) == 0, "isqrt");
static_assert(isqrt(uint128_t(1)) == 1, "isqrt");
static_assert(isqrt(uint128_t(4)) == 2, "isqrt");
static_assert(isqrt(uint128_t(9)) == 3, "isqrt");
static_assert(isqrt(uint128_t(10)) == 3, "isqrt");
static_assert(isqrt(uint128_t(15)) == 3, "isqrt");
static_assert(isqrt(uint128_t(16)) == 4, "isqrt");
static_assert(isqrt(uint128_t(257 * 257)) == 257, "isqrt");
static_assert(isqrt(uint128_t(257 * 257 + 1)) == 257, "isqrt");
static_assert(isqrt(uint128_t(258 * 258 - 1)) == 257, "isqrt");
static_assert(isqrt(uint128_t(1 << 12)) == 1 << 6, "isqrt");
static_assert(isqrt(uint128_t(1 << 14)) == 1 << 7, "isqrt");
static_assert(isqrt(uint128_t(1 << 16)) == 1 << 8, "isqrt");
static_assert(isqrt(uint128_t(1 << 28)) == 1 << 14, "isqrt");
static_assert(isqrt(uint128_t(1 << 30)) == 1 << 15, "isqrt");
static_assert(isqrt(uint128_t(1) << 54) == uint64_t(1) << 27, "isqrt");
static_assert(isqrt(uint128_t(1) << 56) == uint64_t(1) << 28, "isqrt");
static_assert(isqrt(uint128_t(1) << 58) == uint64_t(1) << 29, "isqrt");
static_assert(isqrt(uint128_t(1) << 60) == uint64_t(1) << 30, "isqrt");
static_assert(isqrt(uint128_t(1) << 62) == uint64_t(1) << 31, "isqrt");
static_assert(isqrt(uint128_t(uint64_t(-1))) == (uint64_t(1) << 32) - 1, "isqrt");
static_assert(isqrt(uint128_t(1) << 126) == uint64_t(1) << 63, "isqrt");
static_assert(isqrt(uint128_t(-1)) == (uint128_t(1) << 64) - 1, "isqrt");
static_assert(isqrt(uint128_t(1000000007) * 1000000007) == 1000000007, "isqrt");
static_assert(isqrt(uint128_t(1000000000000000003ull) * 1000000000000000003ull) ==
                  1000000000000000003ull,
              "isqrt");
static_assert(isqrt(uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
                  1000000000000000009ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551521ull) * 18446744073709551521ull) ==
                  18446744073709551521ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551533ull) * 18446744073709551533ull) ==
                  18446744073709551533ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551557ull) * 18446744073709551557ull) ==
                  18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551557ull) * 18446744073709551557ull + 1) ==
                  18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551558ull) * 18446744073709551558ull - 1) ==
                  18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551558ull) * 18446744073709551558ull) ==
                  18446744073709551558ull,
              "isqrt");
#endif

static_assert(icbrt(0u) == 0, "icbrt");
static_assert(icbrt(1u) == 1, "icbrt");
static_assert(icbrt(8u) == 2, "icbrt");
static_assert(icbrt(27u) == 3, "icbrt");
static_assert(icbrt(64u) == 4, "icbrt");
static_assert(icbrt(257u * 257u * 257u) == 257u, "icbrt");
static_assert(icbrt(257u * 257u * 257u + 1) == 257u, "icbrt");
static_assert(icbrt(258u * 258u * 258u - 1) == 257u, "icbrt");
static_assert(icbrt(258u * 258u * 258u) == 258u, "icbrt");
static_assert(icbrt(1u << 15) == 1u << 5, "icbrt");
static_assert(icbrt(1u << 18) == 1u << 6, "icbrt");
static_assert(icbrt(1u << 21) == 1u << 7, "icbrt");
static_assert(icbrt(1u << 24) == 1u << 8, "icbrt");
static_assert(icbrt(1u << 27) == 1u << 9, "icbrt");
static_assert(icbrt(1u << 30) == 1u << 10, "icbrt");
static_assert(icbrt(uint32_t(-1)) == 1625u, "icbrt");

static_assert(icbrt(uint64_t(0)) == 0, "icbrt");
static_assert(icbrt(uint64_t(1)) == 1, "icbrt");
static_assert(icbrt(uint64_t(8)) == 2, "icbrt");
static_assert(icbrt(uint64_t(27)) == 3, "icbrt");
static_assert(icbrt(uint64_t(64)) == 4, "icbrt");
static_assert(icbrt(uint64_t(65)) == 4, "icbrt");
static_assert(icbrt(uint64_t(124)) == 4, "icbrt");
static_assert(icbrt(uint64_t(125)) == 5, "icbrt");
static_assert(icbrt(uint64_t(289) * 289 * 289) == 289, "icbrt");
static_assert(icbrt(uint64_t(289) * 289 * 289 + 1) == 289, "icbrt");
static_assert(icbrt(uint64_t(290) * 290 * 290 - 1) == 289, "icbrt");
static_assert(icbrt(uint64_t(290) * 290 * 290) == 290, "icbrt");
static_assert(icbrt(uint64_t(1) << 30) == 1 << 10, "icbrt");
static_assert(icbrt(uint64_t(1) << 33) == 1 << 11, "icbrt");
static_assert(icbrt(uint64_t(1) << 36) == 1 << 12, "icbrt");
static_assert(icbrt(uint64_t(1) << 39) == 1 << 13, "icbrt");
static_assert(icbrt(uint64_t(1) << 42) == 1 << 14, "icbrt");
static_assert(icbrt(uint64_t(1) << 45) == 1 << 15, "icbrt");
static_assert(icbrt(uint64_t(1) << 48) == 1 << 16, "icbrt");
static_assert(icbrt(uint64_t(1) << 51) == 1 << 17, "icbrt");
static_assert(icbrt(uint64_t(1) << 54) == 1 << 18, "icbrt");
static_assert(icbrt(uint64_t(1) << 57) == 1 << 19, "icbrt");
static_assert(icbrt(uint64_t(1) << 60) == 1 << 20, "icbrt");
static_assert(icbrt(uint64_t(1) << 63) == 1 << 21, "icbrt");
static_assert(icbrt((uint64_t(1) << 63) | (uint64_t(1) << 32)) == 2097152, "icbrt");
static_assert(icbrt(uint64_t(1'367'631'000'000'000ull)) == 111'000, "icbrt");
static_assert(icbrt(uint64_t(1'000'000'000'000'000'000ull)) == 1'000'000, "icbrt");
static_assert(icbrt(uint64_t(1'331'000'000'000'000'000ull)) == 1'100'000, "icbrt");
static_assert(icbrt(uint64_t(8'000'000'000'000'000'000ull)) == 2'000'000, "icbrt");
static_assert(icbrt(uint64_t(15'625'000'000'000'000'000ull)) == 2'500'000, "icbrt");
static_assert(icbrt(uint64_t(-1)) == 2642245, "icbrt");

static_assert(is_perfect_square(uint64_t(0)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(2)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(3)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(4)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(5)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(9)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(15)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(16)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(324)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1 << 16)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1 << 24)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 32), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 40), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 48), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 56), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 60), "is_perfect_square");

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
static_assert(is_perfect_square(uint128_t(0)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(2)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(3)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(4)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(5)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(9)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(15)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(16)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(324)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1 << 16)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1 << 24)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 32), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 40), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 48), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 56), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 60), "is_perfect_square");
#endif

static_assert(bit_reverse(uint8_t(0b00000000)) == 0b00000000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b00000010)) == 0b01000000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b00001100)) == 0b00110000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b10101010)) == 0b01010101, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b01010101)) == 0b10101010, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b11111111)) == 0b11111111, "bit_reverse");

static_assert(bit_reverse(0b00000000'00000000'00000000'00000000u) ==
                  0b00000000'00000000'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'00000000'00000000'00000001u) ==
                  0b10000000'00000000'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b10000000'00000000'00000000'00000000u) ==
                  0b00000000'00000000'00000000'00000001u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'11111111'00000000'00000000u) ==
                  0b00000000'00000000'11111111'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'00000000'11111111'00000000u) ==
                  0b00000000'11111111'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b10101010'10101010'10101010'10101010u) ==
                  0b01010101'01010101'01010101'01010101u,
              "bit_reverse");
static_assert(bit_reverse(0b11111111'00000000'11111111'00000000u) ==
                  0b00000000'11111111'00000000'11111111u,
              "bit_reverse");

static_assert(bit_reverse(uint64_t(
                  0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
                  0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b10000001'00000000'10000001'00000000'10000001'00000000'10000001'00000000ULL)) ==
                  0b00000000'10000001'00000000'10000001'00000000'10000001'00000000'10000001ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b00001111'00000000'11110000'00000000'10101010'00000000'00000000'00000000ULL)) ==
                  0b00000000'00000000'00000000'01010101'00000000'00001111'00000000'11110000ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b00000000'00000000'00000000'10101010'10101010'00000000'00000000'00000000ULL)) ==
                  0b00000000'00000000'00000000'01010101'01010101'00000000'00000000'00000000ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
                  0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b11111111'00000000'11111111'00000000'11111111'00000000'11111111'00000000ULL)) ==
                  0b00000000'11111111'00000000'11111111'00000000'11111111'00000000'11111111ULL,
              "bit_reverse");
static_assert(bit_reverse(uint64_t(
                  0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000ULL)) ==
                  0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111ULL,
              "bit_reverse");

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

static_assert(bit_reverse(uint128_t(0)) == 0, "bit_reverse");
static_assert(bit_reverse(uint128_t(-1)) == uint128_t(-1), "bit_reverse");

#endif

#if __cplusplus >= 202002L
static_assert(int(detail::pop_count_32_software(0u)) == int(std::popcount(0u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(1u)) == int(std::popcount(1u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(2u)) == int(std::popcount(2u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(3u)) == int(std::popcount(3u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(4u)) == int(std::popcount(4u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(0x4788743u)) == int(std::popcount(0x4788743u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(0x2D425B23u)) == int(std::popcount(0x2D425B23u)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(0xFFFFFFFFu - 1)) ==
                  int(std::popcount(0xFFFFFFFFu - 1)),
              "pop_count_32_software");
static_assert(int(detail::pop_count_32_software(0xFFFFFFFFu)) == int(std::popcount(0xFFFFFFFFu)),
              "pop_count_32_software");
#endif

#if __cplusplus >= 202002L
static_assert(int(detail::pop_count_64_software(uint64_t(0))) == int(std::popcount(uint64_t(0))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(1))) == int(std::popcount(uint64_t(1))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(2))) == int(std::popcount(uint64_t(2))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(3))) == int(std::popcount(uint64_t(3))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(4))) == int(std::popcount(uint64_t(4))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0x4788743u))) ==
                  int(std::popcount(uint64_t(0x4788743u))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0x2D425B23u))) ==
                  int(std::popcount(uint64_t(0x2D425B23u))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFu - 1))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFu - 1))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFu))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFu))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0x5873485893484ull))) ==
                  int(std::popcount(uint64_t(0x5873485893484ull))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0x85923489853245ull))) ==
                  int(std::popcount(uint64_t(0x85923489853245ull))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))),
              "pop_count_64_software");
static_assert(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFFFFFFFFFull))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull))),
              "pop_count_64_software");
#endif

#if __cplusplus >= 202002L
static_assert(std::popcount(0u) - std::popcount(0u) == pop_diff(0, 0));
static_assert(int(std::popcount(1u)) - int(std::popcount(0u)) == pop_diff(1, 0));
static_assert(int(std::popcount(0u)) - int(std::popcount(1u)) == pop_diff(0, 1));
static_assert(int(std::popcount(0xABCDEFu)) - int(std::popcount(4u)) == pop_diff(0xABCDEF, 4));
static_assert(int(std::popcount(uint32_t(uint16_t(-1)))) - int(std::popcount(314u)) ==
              pop_diff(uint16_t(-1), 314));
static_assert(int(std::popcount(uint32_t(-1))) - int(std::popcount(0u)) ==
              pop_diff(uint32_t(-1), 0));
static_assert(int(std::popcount(0u)) - int(std::popcount(uint32_t(-1))) ==
              pop_diff(0, uint32_t(-1)));
static_assert(int(std::popcount(uint32_t(-1))) - int(std::popcount(uint32_t(-1))) ==
              pop_diff(uint32_t(-1), uint32_t(-1)));
#endif

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
static_assert(sign(int128_t(0)) == 0, "sign");
static_assert(sign(int128_t(1)) == 1, "sign");
static_assert(sign(int128_t(-1)) == -1, "sign");
static_assert(sign(int128_t(2)) == 1, "sign");
static_assert(sign(int128_t(-2)) == -1, "sign");
static_assert(sign(int128_t(18446744073709551615ull)) == 1, "sign");
static_assert(sign(-int128_t(18446744073709551615ull)) == -1, "sign");
static_assert(sign(int128_t(1) << 63) == 1, "sign");
static_assert(sign(-(int128_t(1) << 63)) == -1, "sign");
static_assert(sign(int128_t(1) << 126) == 1, "sign");
static_assert(sign(-(int128_t(1) << 126)) == -1, "sign");
static_assert(sign(int128_t((uint128_t(1) << 127) - 1)) == 1, "sign");
static_assert(sign(int128_t(-((uint128_t(1) << 127) - 1))) == -1, "sign");
static_assert(sign(int128_t(-(uint128_t(1) << 127))) == -1, "sign");
#endif

static_assert(same_sign(1, 1), "same_sign");
static_assert(same_sign(1, 0), "same_sign");
static_assert(!same_sign(1, -1), "same_sign");
static_assert(same_sign(0, 1), "same_sign");
static_assert(same_sign(0, 0), "same_sign");
static_assert(!same_sign(0, -1), "same_sign");
static_assert(!same_sign(-1, 1), "same_sign");
static_assert(!same_sign(-1, 0), "same_sign");
static_assert(same_sign(-1, -1), "same_sign");

static_assert(same_sign_strict(1, 1), "same_sign_strict");
static_assert(!same_sign_strict(1, 0), "same_sign_strict");
static_assert(!same_sign_strict(1, -1), "same_sign_strict");
static_assert(!same_sign_strict(0, 1), "same_sign_strict");
static_assert(same_sign_strict(0, 0), "same_sign_strict");
static_assert(!same_sign_strict(0, -1), "same_sign_strict");
static_assert(!same_sign_strict(-1, 1), "same_sign_strict");
static_assert(!same_sign_strict(-1, 0), "same_sign_strict");
static_assert(same_sign_strict(-1, -1), "same_sign_strict");

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
static_assert(uabs(int128_t(0)) == 0, "uabs");
static_assert(uabs(int128_t(1)) == 1, "uabs");
static_assert(uabs(int128_t(-1)) == 1, "uabs");
static_assert(uabs(int128_t(4)) == 4, "uabs");
static_assert(uabs(int128_t(-4)) == 4, "uabs");
static_assert(uabs(int128_t(18446744073709551615ull)) == 18446744073709551615ull, "uabs");
static_assert(uabs(-int128_t(18446744073709551615ull)) == 18446744073709551615ull, "uabs");
static_assert(uabs(int128_t(1) << 126) == uint128_t(1) << 126, "uabs");
static_assert(uabs(-(int128_t(1) << 126)) == uint128_t(1) << 126, "uabs");
static_assert(uabs(int128_t((uint128_t(1) << 127) - 1)) == (uint128_t(1) << 127) - 1, "uabs");
static_assert(uabs(int128_t(-((uint128_t(1) << 127) - 1))) == (uint128_t(1) << 127) - 1, "uabs");
static_assert(uabs(int128_t(-(uint128_t(1) << 127))) == uint128_t(1) << 127, "uabs");
#endif

#if __cplusplus >= 202002L

static_assert(sign(std::popcount(0u) - std::popcount(0u)) == sign(pop_cmp(0, 0)), "pop_cmp");
static_assert(sign(std::popcount(1u) - std::popcount(0u)) == sign(pop_cmp(1, 0)), "pop_cmp");
static_assert(sign(std::popcount(0u) - std::popcount(1u)) == sign(pop_cmp(0, 1)), "pop_cmp");
static_assert(sign(std::popcount(0xABCDEFu) - std::popcount(4u)) == pop_cmp(0xABCDEF, 4),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(uint16_t(-1))) - std::popcount(314u)) ==
                  sign(pop_cmp(uint16_t(-1), 314)),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(-1)) - std::popcount(0u)) ==
                  sign(pop_cmp(uint32_t(-1), 0)),
              "pop_cmp");
static_assert(sign(std::popcount(0u) - std::popcount(uint32_t(-1))) ==
                  sign(pop_cmp(0, uint32_t(-1))),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(-1)) - std::popcount(uint32_t(-1))) ==
                  sign(pop_cmp(uint32_t(-1), uint32_t(-1))),
              "pop_cmp");
#endif

static_assert(detail::lz_count_32_software(0) == 32, "lz_count_32_software");
static_assert(detail::lz_count_32_software(1) == 31, "lz_count_32_software");
static_assert(detail::lz_count_32_software(2) == 30, "lz_count_32_software");
static_assert(detail::lz_count_32_software(4) == 29, "lz_count_32_software");
static_assert(detail::lz_count_32_software(8) == 28, "lz_count_32_software");
static_assert(detail::lz_count_32_software(12) == 28, "lz_count_32_software");
static_assert(detail::lz_count_32_software(16) == 27, "lz_count_32_software");
static_assert(detail::lz_count_32_software(32) == 26, "lz_count_32_software");
static_assert(detail::lz_count_32_software(48) == 26, "lz_count_32_software");
static_assert(detail::lz_count_32_software(uint32_t(1) << 30) == 1, "lz_count_32_software");
static_assert(detail::lz_count_32_software(uint32_t(1) << 31) == 0, "lz_count_32_software");
static_assert(detail::lz_count_32_software(~uint32_t(1)) == 0, "lz_count_32_software");

static_assert(detail::lz_count_64_software(0) == 64, "lz_count_64_software");
static_assert(detail::lz_count_64_software(1) == 63, "lz_count_64_software");
static_assert(detail::lz_count_64_software(2) == 62, "lz_count_64_software");
static_assert(detail::lz_count_64_software(4) == 61, "lz_count_64_software");
static_assert(detail::lz_count_64_software(8) == 60, "lz_count_64_software");
static_assert(detail::lz_count_64_software(12) == 60, "lz_count_64_software");
static_assert(detail::lz_count_64_software(16) == 59, "lz_count_64_software");
static_assert(detail::lz_count_64_software(32) == 58, "lz_count_64_software");
static_assert(detail::lz_count_64_software(48) == 58, "lz_count_64_software");
static_assert(detail::lz_count_64_software(uint32_t(1) << 30) == 33, "lz_count_64_software");
static_assert(detail::lz_count_64_software(uint32_t(1) << 31) == 32, "lz_count_64_software");
static_assert(detail::lz_count_64_software(~uint32_t(1)) == 32, "lz_count_64_software");
static_assert(detail::lz_count_64_software(uint64_t(1) << 62) == 1, "lz_count_64_software");
static_assert(detail::lz_count_64_software(uint64_t(1) << 63) == 0, "lz_count_64_software");
static_assert(detail::lz_count_64_software(uint64_t(-1)) == 0, "lz_count_64_software");

static_assert(detail::tz_count_32_software(0u) == 32, "tz_count_32_software");
static_assert(detail::tz_count_32_software(1u) == 0, "tz_count_32_software");
static_assert(detail::tz_count_32_software(2u) == 1, "tz_count_32_software");
static_assert(detail::tz_count_32_software(4u) == 2, "tz_count_32_software");
static_assert(detail::tz_count_32_software(8u) == 3, "tz_count_32_software");
static_assert(detail::tz_count_32_software(12u) == 2, "tz_count_32_software");
static_assert(detail::tz_count_32_software(16u) == 4, "tz_count_32_software");
static_assert(detail::tz_count_32_software(32u) == 5, "tz_count_32_software");
static_assert(detail::tz_count_32_software(48u) == 4, "tz_count_32_software");
static_assert(detail::tz_count_32_software(1u << 30) == 30, "tz_count_32_software");
static_assert(detail::tz_count_32_software(1u << 31) == 31, "tz_count_32_software");
static_assert(detail::tz_count_32_software(~1u) == 1, "tz_count_32_software");
static_assert(detail::tz_count_32_software(uint32_t(-1)) == 0, "tz_count_32_software");

static_assert(detail::tz_count_64_software(0u) == 64, "tz_count_64_software");
static_assert(detail::tz_count_64_software(1u) == 0, "tz_count_64_software");
static_assert(detail::tz_count_64_software(2u) == 1, "tz_count_64_software");
static_assert(detail::tz_count_64_software(4u) == 2, "tz_count_64_software");
static_assert(detail::tz_count_64_software(8u) == 3, "tz_count_64_software");
static_assert(detail::tz_count_64_software(12u) == 2, "tz_count_64_software");
static_assert(detail::tz_count_64_software(16u) == 4, "tz_count_64_software");
static_assert(detail::tz_count_64_software(32u) == 5, "tz_count_64_software");
static_assert(detail::tz_count_64_software(48u) == 4, "tz_count_64_software");
static_assert(detail::tz_count_64_software(1u << 30) == 30, "tz_count_64_software");
static_assert(detail::tz_count_64_software(1u << 31) == 31, "tz_count_64_software");
static_assert(detail::tz_count_64_software(~1u) == 1, "tz_count_64_software");
static_assert(detail::tz_count_64_software(uint32_t(-1)) == 0, "tz_count_64_software");

static_assert(next_n_bits_permutation(0b0010011) == 0b0010101, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0010101) == 0b0010110, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0010110) == 0b0011001, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011001) == 0b0011010, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011010) == 0b0011100, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011100) == 0b0100011, "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0100011) == 0b0100101, "next_n_bits_permutation");

static_assert(next_n_bits_permutation(0b01) == 0b10, "next_n_bits_permutation");

static_assert(next_n_bits_permutation(0b1111111) == 0b10111111, "next_n_bits_permutation");

static_assert(!is_pow2(0ull), "is_pow2");
static_assert(is_pow2(1ull << 0), "is_pow2");
static_assert(is_pow2(1ull << 1), "is_pow2");
static_assert(is_pow2(1ull << 2), "is_pow2");
static_assert(is_pow2(1ull << 3), "is_pow2");
static_assert(is_pow2(1ull << 4), "is_pow2");
static_assert(is_pow2(1ull << 5), "is_pow2");
static_assert(is_pow2(1ull << 6), "is_pow2");
static_assert(is_pow2(1ull << 7), "is_pow2");
static_assert(is_pow2(1ull << 8), "is_pow2");
static_assert(is_pow2(1ull << 9), "is_pow2");
static_assert(is_pow2(1ull << 60), "is_pow2");
static_assert(is_pow2(1ull << 61), "is_pow2");
static_assert(is_pow2(1ull << 62), "is_pow2");
static_assert(is_pow2(1ull << 63), "is_pow2");

#if defined(INTEGERS_128_BIT_HPP)
static_assert(!is_pow2(uint128_t(0)), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 0), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 1), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 2), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 3), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 4), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 5), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 6), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 7), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 8), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 9), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 60), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 61), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 62), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 63), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 64), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 65), "is_pow2");
static_assert(is_pow2(uint128_t(1) << 127), "is_pow2");
#endif

static_assert(nearest_pow2_ge(uint32_t(0u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(2u)) == 2u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(3u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(4u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(5u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(6u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(7u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(8u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(16u)) == 16u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(17u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(18u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(19u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(20u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x7FFFFFFFu)) == 0x80000000u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x80000000u)) == 0x80000000u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x80000001u)) == 0x100000000ull, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0xFFFFFFFFu)) == 0x100000000ull, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 0) == uint32_t(1) << 0, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 1) == uint32_t(1) << 1, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 2) == uint32_t(1) << 2, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 3) == uint32_t(1) << 3, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 4) == uint32_t(1) << 4, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 5) == uint32_t(1) << 5, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 6) == uint32_t(1) << 6, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 7) == uint32_t(1) << 7, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 8) == uint32_t(1) << 8, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 9) == uint32_t(1) << 9, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 10) == uint32_t(1) << 10, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 11) == uint32_t(1) << 11, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 12) == uint32_t(1) << 12, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 13) == uint32_t(1) << 13, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 14) == uint32_t(1) << 14, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 15) == uint32_t(1) << 15, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 16) == uint32_t(1) << 16, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 17) == uint32_t(1) << 17, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 18) == uint32_t(1) << 18, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 19) == uint32_t(1) << 19, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 20) == uint32_t(1) << 20, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 21) == uint32_t(1) << 21, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 22) == uint32_t(1) << 22, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 23) == uint32_t(1) << 23, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 24) == uint32_t(1) << 24, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 25) == uint32_t(1) << 25, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 26) == uint32_t(1) << 26, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 27) == uint32_t(1) << 27, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 28) == uint32_t(1) << 28, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 29) == uint32_t(1) << 29, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 30) == uint32_t(1) << 30, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 31) == uint32_t(1) << 31, "nearest_pow2_ge");

static_assert(nearest_pow2_ge(uint64_t(0u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(2u)) == 2u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(3u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(4u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(5u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(6u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(7u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(8u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(16u)) == 16u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(17u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(18u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(19u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(20u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x7FFFFFFFu)) == 0x80000000u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x80000000u)) == 0x80000000u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x80000001u)) == 0x100000000ull, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0xFFFFFFFFu)) == 0x100000000ull, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x7FFFFFFFFFFFFFFFull)) == 0x8000000000000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x8000000000000000ull)) == 0x8000000000000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 0) == uint64_t(1) << 0, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 1) == uint64_t(1) << 1, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 2) == uint64_t(1) << 2, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 3) == uint64_t(1) << 3, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 4) == uint64_t(1) << 4, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 5) == uint64_t(1) << 5, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 6) == uint64_t(1) << 6, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 7) == uint64_t(1) << 7, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 8) == uint64_t(1) << 8, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 9) == uint64_t(1) << 9, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 10) == uint64_t(1) << 10, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 11) == uint64_t(1) << 11, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 12) == uint64_t(1) << 12, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 13) == uint64_t(1) << 13, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 14) == uint64_t(1) << 14, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 15) == uint64_t(1) << 15, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 16) == uint64_t(1) << 16, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 17) == uint64_t(1) << 17, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 18) == uint64_t(1) << 18, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 19) == uint64_t(1) << 19, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 20) == uint64_t(1) << 20, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 21) == uint64_t(1) << 21, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 22) == uint64_t(1) << 22, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 23) == uint64_t(1) << 23, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 24) == uint64_t(1) << 24, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 25) == uint64_t(1) << 25, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 26) == uint64_t(1) << 26, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 27) == uint64_t(1) << 27, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 28) == uint64_t(1) << 28, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 29) == uint64_t(1) << 29, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 30) == uint64_t(1) << 30, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 31) == uint64_t(1) << 31, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 32) == uint64_t(1) << 32, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 33) == uint64_t(1) << 33, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 34) == uint64_t(1) << 34, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 35) == uint64_t(1) << 35, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 36) == uint64_t(1) << 36, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 37) == uint64_t(1) << 37, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 38) == uint64_t(1) << 38, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 39) == uint64_t(1) << 39, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 40) == uint64_t(1) << 40, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 41) == uint64_t(1) << 41, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 42) == uint64_t(1) << 42, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 43) == uint64_t(1) << 43, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 44) == uint64_t(1) << 44, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 45) == uint64_t(1) << 45, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 46) == uint64_t(1) << 46, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 47) == uint64_t(1) << 47, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 48) == uint64_t(1) << 48, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 49) == uint64_t(1) << 49, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 50) == uint64_t(1) << 50, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 51) == uint64_t(1) << 51, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 52) == uint64_t(1) << 52, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 53) == uint64_t(1) << 53, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 54) == uint64_t(1) << 54, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 55) == uint64_t(1) << 55, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 56) == uint64_t(1) << 56, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 57) == uint64_t(1) << 57, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 58) == uint64_t(1) << 58, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 59) == uint64_t(1) << 59, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 60) == uint64_t(1) << 60, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 61) == uint64_t(1) << 61, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 62) == uint64_t(1) << 62, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 63) == uint64_t(1) << 63, "nearest_pow2_ge");

static_assert(log2_floor(uint32_t(0)) == uint32_t(-1), "log2_floor");
static_assert(log2_floor(uint32_t(1)) == 0, "log2_floor");
static_assert(log2_floor(uint32_t(2)) == 1, "log2_floor");
static_assert(log2_floor(uint32_t(4)) == 2, "log2_floor");
static_assert(log2_floor(uint32_t(8)) == 3, "log2_floor");
static_assert(log2_floor(uint32_t(9)) == 3, "log2_floor");
static_assert(log2_floor(uint32_t(10)) == 3, "log2_floor");
static_assert(log2_floor(uint32_t(15)) == 3, "log2_floor");
static_assert(log2_floor(uint32_t(16)) == 4, "log2_floor");
static_assert(log2_floor(uint32_t(99)) == 6, "log2_floor");
static_assert(log2_floor(uint32_t(100)) == 6, "log2_floor");
static_assert(log2_floor(uint32_t(127)) == 6, "log2_floor");
static_assert(log2_floor(uint32_t(128)) == 7, "log2_floor");
static_assert(log2_floor(uint32_t(129)) == 7, "log2_floor");
static_assert(log2_floor(uint32_t(-1)) == 31, "log2_floor");

static_assert(log2_ceil(uint32_t(0)) == uint32_t(-1), "log2_ceil");
static_assert(log2_ceil(uint32_t(1)) == 0, "log2_ceil");
static_assert(log2_ceil(uint32_t(2)) == 1, "log2_ceil");
static_assert(log2_ceil(uint32_t(4)) == 2, "log2_ceil");
static_assert(log2_ceil(uint32_t(8)) == 3, "log2_ceil");
static_assert(log2_ceil(uint32_t(9)) == 4, "log2_ceil");
static_assert(log2_ceil(uint32_t(10)) == 4, "log2_ceil");
static_assert(log2_ceil(uint32_t(15)) == 4, "log2_ceil");
static_assert(log2_ceil(uint32_t(16)) == 4, "log2_ceil");
static_assert(log2_ceil(uint32_t(99)) == 7, "log2_ceil");
static_assert(log2_ceil(uint32_t(100)) == 7, "log2_ceil");
static_assert(log2_ceil(uint32_t(127)) == 7, "log2_ceil");
static_assert(log2_ceil(uint32_t(128)) == 7, "log2_ceil");
static_assert(log2_ceil(uint32_t(129)) == 8, "log2_ceil");
static_assert(log2_ceil(uint32_t(-1)) == 32, "log2_ceil");

#if __cpp_constexpr >= 202211L && defined(__GNUC__)

static_assert(log10_floor(uint32_t(0)) == uint32_t(-1), "log10_floor");
static_assert(log10_floor(uint32_t(1)) == 0, "log10_floor");
static_assert(log10_floor(uint32_t(9)) == 0, "log10_floor");
static_assert(log10_floor(uint32_t(10)) == 1, "log10_floor");
static_assert(log10_floor(uint32_t(11)) == 1, "log10_floor");
static_assert(log10_floor(uint32_t(99)) == 1, "log10_floor");
static_assert(log10_floor(uint32_t(100)) == 2, "log10_floor");
static_assert(log10_floor(uint32_t(101)) == 2, "log10_floor");
static_assert(log10_floor(uint32_t(1000000000)) == 9, "log10_floor");
static_assert(log10_floor(uint32_t(2000000000)) == 9, "log10_floor");
static_assert(log10_floor(uint32_t(4294967294u)) == 9, "log10_floor");
static_assert(log10_floor(uint32_t(1e8)) == 8, "log10_floor");
static_assert(log10_floor(uint32_t(1e9)) == 9, "log10_floor");
static_assert(log10_floor(uint32_t(-1)) == 9, "log10_floor");

static_assert(log10_floor(uint64_t(0)) == uint32_t(-1), "log10_floor");
static_assert(log10_floor(uint64_t(1)) == 0, "log10_floor");
static_assert(log10_floor(uint64_t(9)) == 0, "log10_floor");
static_assert(log10_floor(uint64_t(10)) == 1, "log10_floor");
static_assert(log10_floor(uint64_t(11)) == 1, "log10_floor");
static_assert(log10_floor(uint64_t(99)) == 1, "log10_floor");
static_assert(log10_floor(uint64_t(100)) == 2, "log10_floor");
static_assert(log10_floor(uint64_t(101)) == 2, "log10_floor");
static_assert(log10_floor(uint64_t(1e8)) == 8, "log10_floor");
static_assert(log10_floor(uint64_t(1e9)) == 9, "log10_floor");
static_assert(log10_floor(uint64_t(1e18)) == 18, "log10_floor");
static_assert(log10_floor(uint64_t(1e19)) == 19, "log10_floor");
static_assert(log10_floor(uint64_t(-1)) == 19, "log10_floor");

static_assert(base_10_len(uint32_t(0)) == 1, "base_10_len");
static_assert(base_10_len(uint32_t(1)) == 1, "base_10_len");
static_assert(base_10_len(uint32_t(9)) == 1, "base_10_len");
static_assert(base_10_len(uint32_t(10)) == 2, "base_10_len");
static_assert(base_10_len(uint32_t(11)) == 2, "base_10_len");
static_assert(base_10_len(uint32_t(99)) == 2, "base_10_len");
static_assert(base_10_len(uint32_t(100)) == 3, "base_10_len");
static_assert(base_10_len(uint32_t(101)) == 3, "base_10_len");
static_assert(base_10_len(uint32_t(1000000000)) == 10, "base_10_len");
static_assert(base_10_len(uint32_t(2000000000)) == 10, "base_10_len");
static_assert(base_10_len(uint32_t(4294967294u)) == 10, "base_10_len");
static_assert(base_10_len(uint32_t(1e8)) == 9, "base_10_len");
static_assert(base_10_len(uint32_t(1e9)) == 10, "base_10_len");
static_assert(base_10_len(uint32_t(-1)) == 10, "log10_floor");

static_assert(base_10_len(uint64_t(0)) == 1, "base_10_len");
static_assert(base_10_len(uint64_t(1)) == 1, "base_10_len");
static_assert(base_10_len(uint64_t(9)) == 1, "base_10_len");
static_assert(base_10_len(uint64_t(10)) == 2, "base_10_len");
static_assert(base_10_len(uint64_t(11)) == 2, "base_10_len");
static_assert(base_10_len(uint64_t(99)) == 2, "base_10_len");
static_assert(base_10_len(uint64_t(100)) == 3, "base_10_len");
static_assert(base_10_len(uint64_t(101)) == 3, "base_10_len");
static_assert(base_10_len(uint64_t(1000000000)) == 10, "base_10_len");
static_assert(base_10_len(uint64_t(2000000000)) == 10, "base_10_len");
static_assert(base_10_len(uint64_t(4294967294u)) == 10, "base_10_len");
static_assert(base_10_len(uint64_t(1e8)) == 9, "base_10_len");
static_assert(base_10_len(uint64_t(1e9)) == 10, "base_10_len");
static_assert(base_10_len(uint64_t(1e18)) == 19, "base_10_len");
static_assert(base_10_len(uint64_t(1e19)) == 20, "base_10_len");
static_assert(base_10_len(uint64_t(-1)) == 20, "log10_floor");

#endif  // __cpp_constexpr >= 202211L && defined(__GNUC__)

#if __cplusplus >= 201703L

static_assert(base_b_len(0ull) == 1, "base_b_len");
static_assert(base_b_len(1ull) == 1, "base_b_len");
static_assert(base_b_len(9ull) == 1, "base_b_len");
static_assert(base_b_len(10ull) == 2, "base_b_len");
static_assert(base_b_len(11ull) == 2, "base_b_len");
static_assert(base_b_len(99ull) == 2, "base_b_len");
static_assert(base_b_len(100ull) == 3, "base_b_len");
static_assert(base_b_len(101ull) == 3, "base_b_len");
static_assert(base_b_len(uint64_t(-1)) == 20, "base_b_len");

#if defined(INTEGERS_128_BIT_HPP)

static_assert(base_b_len(uint128_t(0)) == 1, "base_b_len");
static_assert(base_b_len(uint128_t(1)) == 1, "base_b_len");
static_assert(base_b_len(uint128_t(9)) == 1, "base_b_len");
static_assert(base_b_len(uint128_t(10)) == 2, "base_b_len");
static_assert(base_b_len(uint128_t(11)) == 2, "base_b_len");
static_assert(base_b_len(uint128_t(99)) == 2, "base_b_len");
static_assert(base_b_len(uint128_t(100)) == 3, "base_b_len");
static_assert(base_b_len(uint128_t(101)) == 3, "base_b_len");
static_assert(base_b_len(uint128_t(-1)) == 39, "base_b_len");

#endif  // INTEGERS_128_BIT_HPP

#endif  // __cplusplus >= 201703L

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

static_assert(gcd(uint128_t(1), uint128_t(1)) == 1, "gcd");
static_assert(gcd(uint128_t(3), uint128_t(7)) == 1, "gcd");
static_assert(gcd(uint128_t(0), uint128_t(112378432)) == 112378432, "gcd");
static_assert(gcd(uint128_t(112378432), uint128_t(0)) == 112378432, "gcd");
static_assert(gcd(uint128_t(429384832), uint128_t(324884)) == 4, "gcd");
static_assert(gcd(uint128_t(18446744073709551521ull), uint128_t(18446744073709551533ull)) == 1,
              "gcd");
static_assert(gcd(uint128_t(18446744073709551521ull) * 18446744073709551521ull,
                  uint128_t(18446744073709551521ull)) == 18446744073709551521ull,
              "gcd");
static_assert(gcd(uint128_t(23999993441ull) * 23999993377ull,
                  uint128_t(23999992931ull) * 23999539633ull) == 1,
              "gcd");
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u,
                  uint128_t(2146514611u) * 2146514621u * 2146514647u) == 2146514611ull,
              "gcd");
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u * 2,
                  uint128_t(2146514599u) * 2146514603u * 2146514611u * 3) ==
                  uint128_t(2146514599u) * 2146514603u * 2146514611u,
              "gcd");
static_assert(gcd(uint128_t(100000000000000003ull) * 1000000000000000003ull,
                  uint128_t(1000000000000000003ull) * 1000000000000000009ull) ==
                  1000000000000000003ull,
              "gcd");
static_assert(gcd(uint128_t(3 * 2 * 5 * 7 * 11 * 13 * 17 * 19),
                  uint128_t(18446744073709551557ull) * 3) == 3,
              "gcd");
static_assert(gcd(uint128_t(1000000000000000009ull),
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
                  1000000000000000009ull,
              "gcd");
static_assert(gcd(uint128_t(0), uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull,
              "gcd");
static_assert(gcd(uint128_t(18446744073709551557ull), uint128_t(0)) == 18446744073709551557ull,
              "gcd");

static_assert(gcd(uint64_t(2), int128_t(4)) == 2, "gcd");
static_assert(gcd(uint64_t(2), int128_t(-4)) == 2, "gcd");
static_assert(gcd(uint64_t(3), int128_t(7)) == 1, "gcd");
static_assert(gcd(uint64_t(3), int128_t(-7)) == 1, "gcd");
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * 3) == 3, "gcd");
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * (-3)) == 3, "gcd");
static_assert(gcd(uint64_t(3) * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                  int128_t(18446744073709551557ull) * 3) == 3,
              "gcd");
static_assert(gcd(uint64_t(1000000000000000009ull),
                  int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
                  1000000000000000009ull,
              "gcd");
static_assert(gcd(uint64_t(0), int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
                  uint128_t(1000000000000000009ll) * 1000000000000000009ull,
              "gcd");
static_assert(gcd(uint64_t(18446744073709551557ull), int128_t(0)) == 18446744073709551557ull,
              "gcd");

static_assert(math_functions::popcount(0u) == 0, "popcount");
static_assert(math_functions::popcount(1u << 1) == 1, "popcount");
static_assert(math_functions::popcount(1u << 2) == 1, "popcount");
static_assert(math_functions::popcount(1u << 3) == 1, "popcount");
static_assert(math_functions::popcount(1u << 4) == 1, "popcount");
static_assert(math_functions::popcount(1u << 5) == 1, "popcount");
static_assert(math_functions::popcount(1u << 6) == 1, "popcount");
static_assert(math_functions::popcount(1u << 7) == 1, "popcount");
static_assert(math_functions::popcount(1u << 8) == 1, "popcount");
static_assert(math_functions::popcount(1u << 9) == 1, "popcount");
static_assert(math_functions::popcount(1u << 10) == 1, "popcount");
static_assert(math_functions::popcount(1u << 12) == 1, "popcount");
static_assert(math_functions::popcount(1u << 13) == 1, "popcount");
static_assert(math_functions::popcount(1u << 14) == 1, "popcount");
static_assert(math_functions::popcount(1u << 30) == 1, "popcount");
static_assert(math_functions::popcount(1u << 31) == 1, "popcount");

static_assert(math_functions::popcount(0ul) == 0, "popcount");
static_assert(math_functions::popcount(1ul << 1) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 2) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 3) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 4) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 5) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 6) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 7) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 8) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 9) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 10) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 12) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 13) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 14) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 30) == 1, "popcount");
static_assert(math_functions::popcount(1ul << 31) == 1, "popcount");

static_assert(math_functions::popcount(0ull) == 0, "popcount");
static_assert(math_functions::popcount(1ull << 1) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 2) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 3) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 4) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 5) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 6) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 7) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 8) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 9) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 10) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 12) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 13) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 14) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 30) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 31) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 62) == 1, "popcount");
static_assert(math_functions::popcount(1ull << 63) == 1, "popcount");

static_assert(!bool_median(false, false, false), "bool_median");
static_assert(!bool_median(false, false, true), "bool_median");
static_assert(!bool_median(false, true, false), "bool_median");
static_assert(bool_median(false, true, true), "bool_median");
static_assert(!bool_median(true, false, false), "bool_median");
static_assert(bool_median(true, false, true), "bool_median");
static_assert(bool_median(true, true, false), "bool_median");
static_assert(bool_median(false, true, true), "bool_median");

#endif  // defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

static void test_isqrt() {
    log_tests_started();

    constexpr uint32_t kIters = 1000000;

    constexpr auto test_sqrts = [](uint32_t n, uint32_t n_squared) {
        throw_if_not(n == isqrt(n_squared), "Error isqrt(uint32_t) at n = %" PRIu32 "\n",
                     n_squared);
        throw_if_not(n == isqrt(uint64_t(n_squared)), "Error isqrt(uint64_t) at n = %" PRIu32 "\n",
                     n_squared);
        throw_if_not(n == isqrt(uint128_t(n_squared)),
                     "Error isqrt(uint128_t) at n = %" PRIu32 "\n", n_squared);
    };

    constexpr uint32_t kProbes = 32768;
    for (uint32_t i = 0; i + 1 < std::min(kProbes, 65536u); i++) {
        uint32_t i_square      = i * i;
        uint32_t next_i_square = (i + 1) * (i + 1);
        for (uint32_t j = i_square; j < next_i_square; j++) {
            test_sqrts(i, j);
        }
    }
    test_sqrts(65535u, 65535u * 65535u);
    test_sqrts(65535u, (65536u * 65536u) - 1u);

    for (uint32_t r = uint32_t(0) - kIters; r != 0; r++) {
        uint64_t rs = uint64_t(r) * r;
        throw_if_not(r == isqrt(rs), "Error isqrt(uint64_t) at n = %" PRIu64 "\n", rs);
        throw_if_not(r == isqrt(uint128_t(rs)), "Error isqrt(uint128_t) at n = %" PRIu64 "\n", rs);
    }

    for (uint64_t r = uint64_t(0) - kIters; r != 0; r++) {
        uint128_t rs = uint128_t(r) * r;
        throw_if_not(r == isqrt(uint128_t(rs)), "Error isqrt(uint128_t) at n = %" PRIu64 "\n", rs);
    }
}

static void test_icbrt() noexcept {
    log_tests_started();

    for (uint32_t n = 1; n < 1625; n++) {
        uint32_t tr = n * n * n;
        assert(icbrt(tr) == n);
        assert(icbrt(uint64_t(tr)) == n);
        assert(icbrt(tr + 3 * n * n + 3 * n) == n);
        assert(icbrt(uint64_t(tr + 3 * n * n + 3 * n)) == n);
    }
    assert(icbrt(1625u * 1625u * 1625u) == 1625);
    assert(icbrt(std::numeric_limits<uint32_t>::max()) == 1625);

    for (uint64_t n = 1625; n < 2642245; n++) {
        uint64_t tr = n * n * n;
        assert(icbrt(tr) == n);
        assert(icbrt(tr + 3 * n * n + 3 * n) == n);
    }
    assert(icbrt(uint64_t(2642245) * 2642245 * 2642245) == 2642245);
    assert(icbrt(std::numeric_limits<uint64_t>::max()) == 2642245);
}

static void test_log2() noexcept {
    log_tests_started();

    for (uint32_t k = 0; k < sizeof(uint32_t) * CHAR_BIT; k++) {
        uint32_t pw = uint32_t(1) << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_pow2(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint64_t) * CHAR_BIT; k++) {
        uint64_t pw = uint64_t(1) << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_pow2(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint128_t) * CHAR_BIT; k++) {
        uint128_t pw = uint128_t(1) << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_pow2(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    assert(log2_floor(uint32_t(0)) == uint32_t(-1));
    assert(log2_ceil(uint32_t(0)) == uint32_t(-1));
    assert(log2_floor(uint64_t(0)) == uint32_t(-1));
    assert(log2_ceil(uint64_t(0)) == uint32_t(-1));
    assert(log2_floor(uint128_t(0)) == uint32_t(-1));
    assert(log2_ceil(uint128_t(0)) == uint32_t(-1));
}

static void test_bit_reverse() noexcept {
    log_tests_started();

    for (uint32_t n = 0; n < 256; n++) {
        assert(bit_reverse(uint8_t(n)) == (bit_reverse(n) >> 24));
    }

    constexpr uint32_t shifts[32] = {2,  3,  5,  7,   11,  13,  17,  19,  23,  29, 31,
                                     37, 41, 43, 47,  53,  59,  61,  67,  71,  73, 79,
                                     83, 89, 97, 101, 103, 107, 109, 113, 127, 131};
    uint128_t n                   = uint64_t(-1);
    for (uint32_t k = uint32_t(1e7); k != 0; k--) {
        uint128_t b = (uint128_t(bit_reverse(uint64_t(n))) << 64) | bit_reverse(uint64_t(n >> 64));
        assert(bit_reverse(n) == b);
        n += shifts[k % 32];
    }
}

static const mpfr_rnd_t kRoundMode = mpfr_get_default_rounding_mode();

template <class FloatType>
static SumSinCos<FloatType> call_sum_of_sines_and_cosines(mpfr_t alpha, mpfr_t beta,
                                                          uint32_t n) noexcept {
    if constexpr (std::is_same_v<FloatType, float>) {
        return sum_of_sines_and_cosines(mpfr_get_flt(alpha, kRoundMode),
                                        mpfr_get_flt(beta, kRoundMode), n);
    } else if constexpr (std::is_same_v<FloatType, double>) {
        return sum_of_sines_and_cosines(mpfr_get_d(alpha, kRoundMode), mpfr_get_d(beta, kRoundMode),
                                        n);
    } else {
        return sum_of_sines_and_cosines(mpfr_get_ld(alpha, kRoundMode),
                                        mpfr_get_ld(beta, kRoundMode), n);
    }
}

template <class FloatType>
static std::pair<bool, bool> check_sums_correctness(mpfr_t c_sines_sum, FloatType sines_sum,
                                                    mpfr_t c_cosines_sum, FloatType cosines_sum,
                                                    FloatType eps) noexcept {
    auto cmp_lambda = [](mpfr_t c_sum, FloatType sum, FloatType lambda_eps) noexcept {
        if constexpr (std::is_same_v<FloatType, float> || std::is_same_v<FloatType, double>) {
            mpfr_sub_d(c_sum, c_sum, sum, kRoundMode);
            mpfr_abs(c_sum, c_sum, kRoundMode);
            return mpfr_cmp_d(c_sum, lambda_eps) <= 0;
        } else {
            const long double upper_bound = sum + lambda_eps;
            const long double lower_bound = sum - lambda_eps;
            return 0 <= mpfr_cmp_ld(c_sum, lower_bound) && mpfr_cmp_ld(c_sum, upper_bound) <= 0;
        }
    };

    return {
        cmp_lambda(c_sines_sum, sines_sum, eps),
        cmp_lambda(c_cosines_sum, cosines_sum, eps),
    };
}

template <class FloatType>
static void test_sin_cos_sum_generic() noexcept {
    log_tests_started();

    constexpr uint32_t kMaxN       = 1e2;
    constexpr int64_t k            = 5;
    constexpr uint32_t angle_scale = 10;
    constexpr double angle_start   = bin_pow(double(angle_scale), -k);

    constexpr FloatType kSumEps = []() constexpr noexcept -> FloatType {
        if constexpr (std::is_same_v<FloatType, float>) {
            return 0.4f;
        } else if constexpr (std::is_same_v<FloatType, double>) {
            return 0.0000001;
        } else {
            return 0.0000001L;
        }
    }();

    mpfr_t alpha;
    mpfr_t beta;
    mpfr_t angle;
    mpfr_t c_sines_sum;
    mpfr_t c_cosines_sum;
    mpfr_t angle_sin;
    mpfr_t angle_cos;
    mpfr_init(alpha);
    mpfr_init(beta);
    mpfr_init(angle);
    mpfr_init(c_sines_sum);
    mpfr_init(c_cosines_sum);
    mpfr_init(angle_sin);
    mpfr_init(angle_cos);
    for (uint32_t n = 0; n < kMaxN; n++) {
        mpfr_set_d(alpha, angle_start, kRoundMode);
        for (int32_t alpha_power = -k; alpha_power <= k; alpha_power++) {
            mpfr_set_d(beta, angle_start, kRoundMode);
            for (int32_t beta_power = -k; beta_power <= k; beta_power++) {
                const auto [sines_sum, cosines_sum] =
                    call_sum_of_sines_and_cosines<FloatType>(alpha, beta, n);
                static_assert(
                    std::is_same_v<std::remove_cvref_t<decltype(sines_sum)>, FloatType> &&
                        std::is_same_v<std::remove_cvref_t<decltype(cosines_sum)>, FloatType>,
                    "sum_of_sines_and_cosines return type error");

                mpfr_set_zero(c_sines_sum, 0);
                mpfr_set_zero(c_cosines_sum, 0);
                mpfr_set(angle, alpha, kRoundMode);
                for (uint32_t i = 0; i < n; i++) {
                    mpfr_sin_cos(angle_sin, angle_cos, angle, kRoundMode);
                    mpfr_add(c_sines_sum, c_sines_sum, angle_sin, kRoundMode);
                    mpfr_add(c_cosines_sum, c_cosines_sum, angle_cos, kRoundMode);
                    mpfr_add(angle, angle, beta, kRoundMode);
                }
                const auto [sin_sum_correct, cos_sum_correct] = check_sums_correctness(
                    c_sines_sum, sines_sum, c_cosines_sum, cosines_sum, kSumEps);
                assert(sin_sum_correct);
                assert(cos_sum_correct);

                mpfr_mul_ui(beta, beta, angle_scale, kRoundMode);
            }

            mpfr_mul_ui(alpha, alpha, angle_scale, kRoundMode);
        }
    }
    mpfr_clear(angle_cos);
    mpfr_clear(angle_sin);
    mpfr_clear(c_cosines_sum);
    mpfr_clear(c_sines_sum);
    mpfr_clear(angle);
    mpfr_clear(beta);
    mpfr_clear(alpha);
}

static void test_sin_cos_sum() noexcept {
    log_tests_started();

    test_sin_cos_sum_generic<float>();
    test_sin_cos_sum_generic<double>();
    test_sin_cos_sum_generic<long double>();
}

static void test_visit_all_submasks() noexcept {
    log_tests_started();

    std::vector<uint64_t> vec;
    vec.reserve(128);
    visit_all_submasks(0b10100, [&](uint64_t m) { vec.push_back(m); });
    assert((vec == std::vector<uint64_t>{0b10100, 0b10000, 0b00100}));

    vec.clear();
    visit_all_submasks(0, [&](uint64_t m) { vec.push_back(m); });
    assert(vec.size() == 1 && vec[0] == 0);

    vec.clear();
    visit_all_submasks(0b111, [&](uint64_t m) { vec.push_back(m); });
    assert((vec == std::vector<uint64_t>{0b111, 0b110, 0b101, 0b100, 0b011, 0b010, 0b001}));
}

int main() {
    test_isqrt();
    test_icbrt();
    test_log2();
    test_bit_reverse();
    test_sin_cos_sum();
    test_visit_all_submasks();
}
