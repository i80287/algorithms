// For CLOCK_REALTIME
#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "../misc/config_macros.hpp"
#include "memset_int.h"

const size_t kTests      = 32;
const size_t kBufferSize = 10000000;

ATTRIBUTE_NOINLINE
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_NODISCARD_WITH_MESSAGE("returned time should be used")
uint64_t measure_memset(int32_t* const buffer, const int32_t value) {
    struct timespec start;
    struct timespec end;
    uint64_t avg_time = 0;
    for (size_t iter = kTests; iter != 0; iter--) {
        clock_gettime(CLOCK_REALTIME, &start);
        memset(buffer, value, kBufferSize * sizeof(int32_t));
        clock_gettime(CLOCK_REALTIME, &end);
        avg_time += (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000 +
                    (uint32_t)(end.tv_nsec - start.tv_nsec);
    }

    return avg_time / kTests;
}

ATTRIBUTE_NOINLINE
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_NODISCARD_WITH_MESSAGE("returned time should be used")
uint64_t measure_memset_int(int32_t* const buffer, const int32_t value) {
    struct timespec start;
    struct timespec end;
    uint64_t avg_time = 0;
    for (size_t iter = kTests; iter != 0; iter--) {
        clock_gettime(CLOCK_REALTIME, &start);
        memset_int(buffer, value, kBufferSize);
        clock_gettime(CLOCK_REALTIME, &end);
        avg_time += (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000 +
                    (uint32_t)(end.tv_nsec - start.tv_nsec);
    }

    return avg_time / kTests;
}

int main(void) {
    int32_t* const buffer = (int32_t*)calloc(kBufferSize, sizeof(int32_t));
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }

    const int32_t value = atoi("255");

    do_not_optimize_away(measure_memset(buffer, value));
    do_not_optimize_away(measure_memset_int(buffer, value));
    do_not_optimize_away(measure_memset(buffer, value));
    do_not_optimize_away(measure_memset_int(buffer, value));

    const uint64_t time1 = measure_memset_int(buffer, value);
    const uint64_t time2 = measure_memset(buffer, value);

    do_not_optimize_away(measure_memset(buffer, value));
    do_not_optimize_away(measure_memset_int(buffer, value));
    do_not_optimize_away(measure_memset(buffer, value));
    do_not_optimize_away(measure_memset_int(buffer, value));

    const uint64_t time3 = measure_memset(buffer, value);
    const uint64_t time4 = measure_memset_int(buffer, value);

    // clang-format off
    int ret = printf(
        "memset_int:\n"
        "  test 1: %" PRIu64 " ns\n"
        "  test 2: %" PRIu64 " ns\n"
        "  avrg  : %" PRIu64 " ns\n"
        "memset:\n"
        "  test 1: %" PRIu64 " ns\n"
        "  test 2: %" PRIu64 " ns\n"
        "  avrg  : %" PRIu64 " ns\n",
        time1, time4, (time1 + time4) / 2, time2, time3, (time2 + time3) / 2);
    // clang-format on

    free(buffer);
    return ret > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/**
 * Possible outputs:
 *
 * memset_int:
 *   test 1: 3749987 ns
 *   test 2: 3750450 ns
 *   avrg  : 3750218 ns
 * memset:
 *   test 1: 3749978 ns
 *   test 2: 3749534 ns
 *   avrg  : 3749756 ns
 *
 * memset_int:
 *   test 1: 3687525 ns
 *   test 2: 3687440 ns
 *   avrg  : 3687482 ns
 * memset:
 *   test 1: 3687481 ns
 *   test 2: 3687559 ns
 *   avrg  : 3687520 ns
 *
 * memset_int:
 *   test 1: 3718731 ns
 *   test 2: 3718750 ns
 *   avrg  : 3718740 ns
 * memset:
 *   test 1: 3718962 ns
 *   test 2: 3718737 ns
 *   avrg  : 3718849 ns
 *
 *-----------------------
 *
 * memset_int:
 *   test 1: 3718728 ns
 *   test 2: 3719115 ns
 *   avrg  : 3718921 ns
 * memset:
 *   test 1: 1312503 ns
 *   test 2: 1343381 ns
 *   avrg  : 1327942 ns
 *
 * memset_int:
 *   test 1: 3749984 ns
 *   test 2: 3730318 ns
 *   avrg  : 3740151 ns
 * memset:
 *   test 1: 1281240 ns
 *   test 2: 1312409 ns
 *   avrg  : 1296824 ns
 *
 *
 *-----------------------
 *
 * memset_int:
 *   test 1: 3670030 ns
 *   test 2: 3668985 ns
 *   avrg  : 3669507 ns
 * memset:
 *   test 1: 3679393 ns
 *   test 2: 3690278 ns
 *   avrg  : 3684835 ns
 *
 * memset_int:
 *   test 1: 3677818 ns
 *   test 2: 3668477 ns
 *   avrg  : 3673147 ns
 * memset:
 *   test 1: 3686079 ns
 *   test 2: 3687984 ns
 *   avrg  : 3687031 ns
 *
 */
