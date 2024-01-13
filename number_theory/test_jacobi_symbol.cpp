#include <cassert>
#include <cinttypes>

#include "jacobi_symbol.hpp"

// Link with -lgmp and -lgmpxx
#include <gmpxx.h>

static const int32_t jcb[30][30] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, -1, 0,  -1, 0,  1, 0, 1, 0, -1, 0,  -1, 0,  1,
     0, 1, 0,  -1, 0,  -1, 0, 1, 0, 1, 0,  -1, 0,  -1, 0},
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    {1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0,
     1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0},
    {1, 0,  0, 0,  1, 0, 1, 0,  0, 0, 1, 0, -1, 0, 0,
     0, -1, 0, -1, 0, 0, 0, -1, 0, 1, 0, 0, 0,  1, 0},
    {1, 1,  -1, 1,  -1, -1, 0, 1, 1,  -1, 1,  -1, -1, 0, 1,
     1, -1, 1,  -1, -1, 0,  1, 1, -1, 1,  -1, -1, 0,  1, 1},
    {1, 0, -1, 0,  -1, 0,  1, 0, 1, 0, -1, 0,  -1, 0,  1,
     0, 1, 0,  -1, 0,  -1, 0, 1, 0, 1, 0,  -1, 0,  -1, 0},
    {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0,
     1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},
    {1, 0,  1, 0,  0, 0,  -1, 0,  1, 0, -1, 0, 1, 0,  0,
     0, -1, 0, -1, 0, -1, 0,  -1, 0, 0, 0,  1, 0, -1, 0},
    {1, -1, 1,  1,  1, -1, -1, -1, 1,  -1, 0, 1, -1, 1,  1,
     1, -1, -1, -1, 1, -1, 0,  1,  -1, 1,  1, 1, -1, -1, -1},
    {1, 0,  0, 0, -1, 0, 1, 0,  0, 0, -1, 0, 1, 0,  0,
     0, -1, 0, 1, 0,  0, 0, -1, 0, 1, 0,  0, 0, -1, 0},
    {1, -1, 1,  1,  -1, -1, -1, -1, 1,  1, -1, 1, 0,  1, -1,
     1, 1,  -1, -1, -1, -1, 1,  1,  -1, 1, 0,  1, -1, 1, 1},
    {1, 0,  1, 0, 1, 0, 0, 0, 1, 0, -1, 0, 1, 0,  1,
     0, -1, 0, 1, 0, 0, 0, 1, 0, 1, 0,  1, 0, -1, 0},
    {1, 1, 0, 1, 0, 0, -1, 1, 0, 0, -1, 0, -1, -1, 0,
     1, 1, 0, 1, 0, 0, -1, 1, 0, 0, -1, 0, -1, -1, 0},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
     0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, -1, -1, 1,  -1, 1,
     1, 0, 1,  1, -1, 1,  -1, -1, -1, 1,  1,  -1, -1, -1, 1},
    {1, 0, 0, 0,  -1, 0, 1, 0, 0, 0, -1, 0, -1, 0,  0,
     0, 1, 0, -1, 0,  0, 0, 1, 0, 1, 0,  0, 0,  -1, 0},
    {1, -1, -1, 1, 1, 1,  1,  -1, 1, -1, 1, -1, -1, -1, -1,
     1, 1,  -1, 0, 1, -1, -1, 1,  1, 1,  1, -1, 1,  -1, 1},
    {1, 0,  -1, 0, 0, 0, -1, 0,  1, 0, 1, 0,  -1, 0, 0,
     0, -1, 0,  1, 0, 1, 0,  -1, 0, 0, 0, -1, 0,  1, 0},
    {1, -1, 0, 1,  1, 0, 0, -1, 0, -1, -1, 0, -1, 0,  0,
     1, 1,  0, -1, 1, 0, 1, -1, 0, 1,  1,  0, 0,  -1, 0},
    {1, 0,  -1, 0, -1, 0, -1, 0, 1, 0, 0, 0,  1, 0, 1,
     0, -1, 0,  1, 0,  1, 0,  1, 0, 1, 0, -1, 0, 1, 0},
    {1, 1,  1, 1,  -1, 1,  -1, 1, 1, -1, -1, 1, 1,  -1, -1,
     1, -1, 1, -1, -1, -1, -1, 0, 1, 1,  1,  1, -1, 1,  -1},
    {1, 0,  0, 0,  1, 0, 1, 0,  0, 0, 1, 0, -1, 0, 0,
     0, -1, 0, -1, 0, 0, 0, -1, 0, 1, 0, 0, 0,  1, 0},
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,
     1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {1, 0, -1, 0, 1, 0, -1, 0, 1, 0, 1, 0,  0, 0,  -1,
     0, 1, 0,  1, 0, 1, 0,  1, 0, 1, 0, -1, 0, -1, 0},
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, 0,  -1, 0,  -1, 0, 0, 0, 1, 0, 1, 0,  -1, 0, 1,
     0, -1, 0,  -1, 0,  0, 0, 1, 0, 1, 0, -1, 0,  1, 0},
    {1, -1, -1, 1,  1, 1,  1, -1, 1, -1, -1, -1, 1, -1, -1,
     1, -1, -1, -1, 1, -1, 1, 1,  1, 1,  -1, -1, 1, 0,  1},
    {1, 0, 0, 0,  0, 0, -1, 0, 0, 0, 1, 0, 1, 0, 0,
     0, 1, 0, -1, 0, 0, 0,  1, 0, 0, 0, 0, 0, 1, 0},
};

static const uint32_t primes[30] = {
    3,  5,  7,  11, 13, 17, 19, 23, 29, 31,  37,  41,  43,  47,  53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127};
static const int32_t lgnr[30][30] = {
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0,
     1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0},
    {1, 1,  -1, 1,  -1, -1, 0, 1, 1,  -1, 1,  -1, -1, 0, 1,
     1, -1, 1,  -1, -1, 0,  1, 1, -1, 1,  -1, -1, 0,  1, 1},
    {1, -1, 1,  1,  1, -1, -1, -1, 1,  -1, 0, 1, -1, 1,  1,
     1, -1, -1, -1, 1, -1, 0,  1,  -1, 1,  1, 1, -1, -1, -1},
    {1, -1, 1,  1,  -1, -1, -1, -1, 1,  1, -1, 1, 0,  1, -1,
     1, 1,  -1, -1, -1, -1, 1,  1,  -1, 1, 0,  1, -1, 1, 1},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, -1, -1, 1,  -1, 1,
     1, 0, 1,  1, -1, 1,  -1, -1, -1, 1,  1,  -1, -1, -1, 1},
    {1, -1, -1, 1, 1, 1,  1,  -1, 1, -1, 1, -1, -1, -1, -1,
     1, 1,  -1, 0, 1, -1, -1, 1,  1, 1,  1, -1, 1,  -1, 1},
    {1, 1,  1, 1,  -1, 1,  -1, 1, 1, -1, -1, 1, 1,  -1, -1,
     1, -1, 1, -1, -1, -1, -1, 0, 1, 1,  1,  1, -1, 1,  -1},
    {1, -1, -1, 1,  1, 1,  1, -1, 1, -1, -1, -1, 1, -1, -1,
     1, -1, -1, -1, 1, -1, 1, 1,  1, 1,  -1, -1, 1, 0,  1},
    {1, 1,  -1, 1, 1, -1, 1,  1,  1,  1, -1, -1, -1, 1,  -1,
     1, -1, 1,  1, 1, -1, -1, -1, -1, 1, -1, -1, 1,  -1, -1},
    {1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1, 1, 1, -1, -1, -1,
     1, -1, -1, -1, -1, 1,  -1, -1, -1, 1, 1, 1, 1,  -1, 1},
    {1, 1,  -1, 1,  1, -1, -1, 1, 1,  1, -1, -1, -1, -1, -1,
     1, -1, 1,  -1, 1, 1,  -1, 1, -1, 1, -1, -1, -1, -1, -1},
    {1, -1, -1, 1,  -1, 1, -1, -1, 1, 1, 1,  -1, 1,  1,  1,
     1, 1,  -1, -1, -1, 1, -1, 1,  1, 1, -1, -1, -1, -1, -1},
    {1, 1, 1, 1,  -1, 1, 1,  1,  1, -1, -1, 1, -1, 1,  -1,
     1, 1, 1, -1, -1, 1, -1, -1, 1, 1,  -1, 1, 1,  -1, -1},
    {1, -1, -1, 1,  -1, 1,  1,  -1, 1, 1, 1,  -1, 1, -1, 1,
     1, 1,  -1, -1, -1, -1, -1, -1, 1, 1, -1, -1, 1, 1,  -1},
    {1, -1, 1,  1, 1, -1, 1, -1, 1,  -1, -1, 1, -1, -1, 1,
     1, 1,  -1, 1, 1, 1,  1, -1, -1, 1,  1,  1, 1,  1,  -1},
    {1, -1, 1,  1, 1, -1, -1, -1, 1,  -1, -1, 1, 1,  1,  1,
     1, -1, -1, 1, 1, -1, 1,  -1, -1, 1,  -1, 1, -1, -1, -1},
    {1, -1, -1, 1, -1, 1, -1, -1, 1, 1, -1, -1, -1, 1, 1,
     1, 1,  -1, 1, -1, 1, 1,  1,  1, 1, 1,  -1, -1, 1, -1},
    {1, 1,  1, 1, 1, 1,  -1, 1,  1, 1, -1, 1, -1, -1, 1,
     1, -1, 1, 1, 1, -1, -1, -1, 1, 1, -1, 1, -1, 1,  1},
    {1, 1,  1, 1, -1, 1,  -1, 1, 1, -1, -1, 1, -1, -1, -1,
     1, -1, 1, 1, -1, -1, -1, 1, 1, 1,  -1, 1, -1, -1, -1},
    {1, 1,  -1, 1, 1, -1, -1, 1, 1,  1, 1, -1, 1,  -1, -1,
     1, -1, 1,  1, 1, 1,  1,  1, -1, 1, 1, -1, -1, -1, -1},
    {1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1, 1, 1, -1, -1, -1,
     1, 1,  -1, -1, -1, 1,  -1, 1,  -1, 1, 1, 1, 1,  1,  1},
    {1, 1, -1, 1,  1, -1, -1, 1,  1,  1, 1,  -1, -1, -1, -1,
     1, 1, 1,  -1, 1, 1,  1,  -1, -1, 1, -1, -1, -1, -1, -1},
    {1, 1,  1, 1,  -1, 1,  -1, 1,  1, -1, 1,  1, -1, -1, -1,
     1, -1, 1, -1, -1, -1, 1,  -1, 1, 1,  -1, 1, -1, -1, -1},
    {1, -1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1,  1,  -1,
     1, 1,  -1, 1, 1, 1, 1,  1,  1, 1,  -1, -1, -1, -1, 1},
    {1, 1, -1, 1, -1, -1, 1,  1, 1,  -1, -1, -1, 1, 1, 1,
     1, 1, 1,  1, -1, -1, -1, 1, -1, 1,  1,  -1, 1, 1, 1},
    {1, -1, 1,  1, -1, -1, -1, -1, 1,  1, 1,  1, 1,  1, -1,
     1, -1, -1, 1, -1, -1, -1, 1,  -1, 1, -1, 1, -1, 1, 1},
    {1, -1, 1,  1,  1, -1, 1, -1, 1,  -1, -1, 1, -1, -1, 1,
     1, -1, -1, -1, 1, 1,  1, -1, -1, 1,  1,  1, 1,  1,  -1},
    {1, 1,  -1, 1,  -1, -1, 1, 1,  1,  -1, 1, -1, 1, 1,  1,
     1, -1, 1,  -1, -1, -1, 1, -1, -1, 1,  1, -1, 1, -1, 1},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, 1, -1, 1,  -1, 1,
     1, 1, 1,  1, -1, 1,  1,  -1, -1, 1,  1, -1, -1, -1, 1},
};

static void CheckJacobi(int32_t i, int32_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRId32 ", %" PRId32
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(int64_t i, int64_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRId64 ", %" PRId64
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(uint32_t i, uint32_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRIu32 ", %" PRIu32
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(uint64_t i, uint64_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRIu64 ", %" PRIu64
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(uint32_t i, int32_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRIu32 ", %" PRId32
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(int32_t i, uint32_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRId32 ", %" PRIu32
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(uint64_t i, int64_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRIu64 ", %" PRId64
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

static void CheckJacobi(int64_t i, uint64_t j, const mpz_class& n1,
                        const mpz_class& n2) noexcept {
    int32_t func_jac = JacobiSymbol(i, j);
    int real_jac = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        printf("Error at (%" PRId64 ", %" PRIu64
               "): given J = %d, correct J = %d\n",
               i, j, func_jac, real_jac);
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiI32() {
    mpz_class n1;
    mpz_class n2;

    for (int32_t i = INT32_MIN; i <= int32_t(kLen) + INT32_MIN; i++) {
        n1 = i;
        for (int32_t j = INT32_MIN; j <= int32_t(kLen) + INT32_MIN; j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }

    for (int32_t i = INT32_MAX; i >= INT32_MAX - int32_t(kLen); i--) {
        n1 = i;
        for (int32_t j = INT32_MAX; j >= INT32_MAX - int32_t(kLen); j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiI64() {
    mpz_class n1;
    mpz_class n2;

    n1.set_str("-9223372036854775808", 10);
    for (int64_t i = INT64_MIN; i <= INT64_MIN + int64_t(kLen); ++i, ++n1) {
        n2.set_str("-9223372036854775808", 10);
        for (int64_t j = INT64_MIN; j <= INT64_MIN + int64_t(kLen); ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }

    n1.set_str("9223372036854775807", 10);
    for (int64_t i = INT64_MAX; i >= INT64_MAX - int64_t(kLen); --i, --n1) {
        n2.set_str("9223372036854775807", 10);
        for (int64_t j = INT64_MAX; j >= INT64_MAX - int64_t(kLen); --j, --n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiU32() {
    mpz_class n1;
    mpz_class n2;

    for (uint32_t i = 0; i <= uint32_t(kLen); i++) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t(kLen); j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }

    for (uint32_t i = UINT32_MAX; i >= UINT32_MAX - uint32_t(kLen); i--) {
        n1 = i;
        for (uint32_t j = UINT32_MAX; j >= UINT32_MAX - uint32_t(kLen); j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiU64() {
    mpz_class n1;
    mpz_class n2;

    n1 = 0u;
    for (uint64_t i = 0; i <= uint64_t(kLen); ++i, ++n1) {
        n2 = 0u;
        for (uint64_t j = 0; j <= uint64_t(kLen); ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }

    n1.set_str("18446744073709551615", 10);
    for (uint64_t i = UINT64_MAX; i >= UINT64_MAX - uint64_t(kLen); --i, --n1) {
        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = UINT64_MAX; j >= UINT64_MAX - uint64_t(kLen); --j, --n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiU32I32() {
    mpz_class n1;
    mpz_class n2;

    for (int32_t i = INT32_MIN; i <= int32_t(kLen) + INT32_MIN; i++) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t(kLen); j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
        for (uint32_t j = UINT32_MAX; j >= UINT32_MAX - uint32_t(kLen); j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }

    for (int32_t i = INT32_MAX; i >= INT32_MAX - int32_t(kLen); i--) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t(kLen); j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
        for (uint32_t j = UINT32_MAX; j >= UINT32_MAX - uint32_t(kLen); j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }
}

template <uint16_t kLen>
static void GMPCheckJacobiU64I64() {
    mpz_class n1;
    mpz_class n2;

    n1.set_str("-9223372036854775808", 10);
    for (int64_t i = INT64_MIN; i <= INT64_MIN + int64_t(kLen); ++i, ++n1) {
        n2 = 0u;
        for (uint64_t j = 0; j <= uint64_t(kLen); ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }

        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = UINT64_MAX; j >= UINT64_MAX - uint64_t(kLen); --j, --n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }

    n1.set_str("9223372036854775807", 10);
    for (int64_t i = INT64_MAX; i >= INT64_MAX - int64_t(kLen); --i, --n1) {
        n2 = 0u;
        for (uint64_t j = 0; j <= uint64_t(kLen); ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }

        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = UINT64_MAX; j >= UINT64_MAX - uint64_t(kLen); --j, --n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }
}

int main() {
    for (uint32_t n = 1; n <= 30; n++) {
        for (uint32_t k = 1; k <= 30; k++) {
            int32_t j = jcb[n - 1][k - 1];
            assert(JacobiSymbol(k, n) == j);

            assert(JacobiSymbol(int64_t(k), int64_t(n)) == j);
            assert(JacobiSymbol(uint64_t(k), int64_t(n)) == j);
            assert(JacobiSymbol(int64_t(k), uint64_t(n)) == j);
            assert(JacobiSymbol(uint64_t(k), uint64_t(n)) == j);

            assert(JacobiSymbol(k, -int32_t(n)) == j);
            assert(JacobiSymbol(int64_t(k), int64_t(-int32_t(n))) == j);
            assert(JacobiSymbol(uint64_t(k), int64_t(-int32_t(n))) == j);
        }
    }

    for (int32_t k = -100; k <= 100; k++) {
        const int32_t b = k == 1 || k == -1;
        assert(JacobiSymbol(k, 0) == b);
        assert(JacobiSymbol(k, uint32_t(0)) == b);
        assert(JacobiSymbol(int64_t(k), int64_t(0)) == b);
        assert(JacobiSymbol(int64_t(k), uint64_t(0)) == b);
    }

    for (size_t i = 0; i < 30; i++) {
        uint32_t p = primes[i];
        for (uint32_t a = 1; a <= 30; a++) {
            int32_t l = lgnr[i][a - 1];
            assert(JacobiSymbol(a, p) == l);

            assert(JacobiSymbol(int64_t(a), int64_t(p)) == l);
            assert(JacobiSymbol(uint64_t(a), int64_t(p)) == l);
            assert(JacobiSymbol(int64_t(a), uint64_t(p)) == l);
            assert(JacobiSymbol(uint64_t(a), uint64_t(p)) == l);

            assert(JacobiSymbol(a, -int32_t(p)) == l);
            assert(JacobiSymbol(int64_t(a), int64_t(-int32_t(p))) == l);
            assert(JacobiSymbol(uint64_t(a), int64_t(-int32_t(p))) == l);
        }
    }

    constexpr uint16_t kLen = 2048;
    GMPCheckJacobiI32<kLen>();
    GMPCheckJacobiI64<kLen>();
    GMPCheckJacobiU32<kLen>();
    GMPCheckJacobiU64<kLen>();
    GMPCheckJacobiU32I32<kLen>();
    GMPCheckJacobiU64I64<kLen>();
}
