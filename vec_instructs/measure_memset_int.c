// For CLOCK_REALTIME
#define _POSIX_C_SOURCE 200809L

#include "memset_int.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include <sys/time.h>
#include <sys/types.h>

const size_t kTests = 32;
const size_t kBufferSize = 10000000;

__attribute__((__noinline__)) uint64_t measure_memset(int32_t* buffer, int32_t value) {
    struct timespec start;
    struct timespec end;
    uint64_t avg_time = 0;
    for (size_t iter = kTests; iter != 0; iter--) {
        clock_gettime(CLOCK_REALTIME, &start);
        memset(buffer, value, kBufferSize * sizeof(int32_t));
        clock_gettime(CLOCK_REALTIME, &end);
        avg_time += (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000 + (uint32_t)(end.tv_nsec - start.tv_nsec);
    }

    return avg_time / kTests;
}

__attribute__((__noinline__)) uint64_t measure_memset_int(int32_t* buffer, int32_t value) {
    struct timespec start;
    struct timespec end;
    uint64_t avg_time = 0;
    for (size_t iter = kTests; iter != 0; iter--) {
        clock_gettime(CLOCK_REALTIME, &start);
        memset_int(buffer, value, kBufferSize);
        clock_gettime(CLOCK_REALTIME, &end);
        avg_time += (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000 + (uint32_t)(end.tv_nsec - start.tv_nsec);
    }

    return avg_time / kTests;
}

int main(void) {
    int32_t* buffer = (int32_t*)malloc(kBufferSize * sizeof(int32_t));
    if (buffer == NULL)
        return EXIT_FAILURE;
    int32_t value = atoi("255");

    measure_memset(buffer, value);
    measure_memset_int(buffer, value);
    measure_memset(buffer, value);
    measure_memset_int(buffer, value);

    uint64_t time1 = measure_memset_int(buffer, value);
    uint64_t time2 = measure_memset(buffer, value);

    measure_memset(buffer, value);
    measure_memset_int(buffer, value);
    measure_memset(buffer, value);
    measure_memset_int(buffer, value);

    uint64_t time3 = measure_memset(buffer, value);
    uint64_t time4 = measure_memset_int(buffer, value);

    printf(
        "memset_int:\n"
        "  test 1: %" PRIu64 " ns\n"
        "  test 2: %" PRIu64 " ns\n"
        "  avrg  : %" PRIu64 " ns\n"
        "memset:\n"
        "  test 1: %" PRIu64 " ns\n"
        "  test 2: %" PRIu64 " ns\n"
        "  avrg  : %" PRIu64 " ns\n",
        time1, time4, (time1 + time4) / 2,
        time2, time3, (time2 + time3) / 2);

    free(buffer);
    return EXIT_SUCCESS;
}

// gcc -O2 .\measure_memset_int.c -Wall -Wextra -Wpedantic -Werror -Wunused -pedantic-errors -Wconversion -Wshadow -Wnull-dereference -Wundef -Wwrite-strings -Wbad-function-cast -Wsign-conversion -Wmissing-noreturn -Wunreachable-code -Warray-bounds -o measure_memset_int.exe

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
 */
