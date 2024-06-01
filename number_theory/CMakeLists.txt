cmake_minimum_required(VERSION 3.5)

project(math_tests VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TestExecutables)

add_executable(eratosthene_sieve_test test_eratosthene_sieve.cpp)
list(APPEND TestExecutables eratosthene_sieve_test)

add_executable(fibonacci_num_test test_fibonacci_num.cpp)
list(APPEND TestExecutables fibonacci_num_test)

add_executable(gosper_algorithm_test test_gosper_algorithm.cpp)
list(APPEND TestExecutables gosper_algorithm_test)

add_executable(integers_128_bit_test test_integers_128_bit.cpp)
list(APPEND TestExecutables integers_128_bit_test)

add_executable(is_prime_bpsw_test test_is_prime_bpsw.cpp)
list(APPEND TestExecutables is_prime_bpsw_test)

add_executable(kronecker_symbol_test test_kronecker_symbol.cpp)
list(APPEND TestExecutables kronecker_symbol_test)

add_executable(long_int_test test_long_int.cpp)
list(APPEND TestExecutables long_int_test)

add_executable(math_functions_test test_math_functions.cpp)
list(APPEND TestExecutables math_functions_test)

# Empty by default
set(TEST_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
            set(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} -UNDEBUG)
        endif()
        set(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} _LIBCPP_ENABLE_ASSERTIONS=1)
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "AppleClang")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(TEST_COMPILE_OPTIONS ${TEST_COMPILE_OPTIONS} -UNDEBUG)
    endif()

    set(TEST_COMPILE_OPTIONS
        ${TEST_COMPILE_OPTIONS}
        -fstack-protector-all
        -mshstk
        -fno-omit-frame-pointer
        -Wall
        -Wextra
        -Wlogical-op
        -Wcast-qual
        -Wpedantic
        -Wshift-overflow=2
        -Wduplicated-cond
        -Wunused
        -Wconversion
        -Wunsafe-loop-optimizations
        -Wshadow
        -Wnull-dereference
        -Wundef
        -Wwrite-strings
        -Wsign-conversion
        -Wmissing-noreturn
        -Wunreachable-code
        -Wcast-align
        -Warray-bounds=2
        -Wformat=2)

    set(TEST_COMPILE_DEFINITIONS
        ${TEST_COMPILE_DEFINITIONS}
        _GLIBCXX_DEBUG
        _GLIBCXX_DEBUG_PEDANTIC
        _GLIBCXX_CONCEPT_CHECKS)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

foreach(target ${TestExecutables})
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    target_link_libraries(${target} gmp gmpxx mpfr)
    target_compile_options(${target} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target} PRIVATE ${TEST_COMPILE_DEFINITIONS})
endforeach()

enable_testing()
