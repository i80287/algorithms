cmake_minimum_required(VERSION 3.5)

project(math_tests VERSION 0.1.0 LANGUAGES CXX)

set(TestFilenames)
list(APPEND TestFilenames test_fibonacci_num)
list(APPEND TestFilenames test_gosper_algorithm)
list(APPEND TestFilenames test_integers_128_bit)
list(APPEND TestFilenames test_is_prime_bpsw)
list(APPEND TestFilenames test_kronecker_symbol)
list(APPEND TestFilenames test_long_int)
list(APPEND TestFilenames test_math_functions)
list(APPEND TestFilenames test_bitmatrix)

# Empty by default
set(TEST_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

function(configure_gcc_or_clang_gcc_options)
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${TEST_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${TEST_COMPILE_DEFINITIONS})
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        -fstack-protector-strong
        -fno-omit-frame-pointer
        -mshstk
        -Wall
        -Wextra
        -Wcast-qual
        -Wpedantic
        -Wshift-overflow
        -Wunused
        -Wconversion
        -Wshadow
        -Wnull-dereference
        -Wundef
        -Wwrite-strings
        -Wsign-conversion
        -Wmissing-noreturn
        -Wunreachable-code
        -Wcast-align
        -Warray-bounds
        -Wformat=2)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            -Wlogical-op
            -Wunsafe-loop-optimizations
            -Wduplicated-cond)
    endif()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _LIBCPP_ENABLE_ASSERTIONS=1)
    endif()
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        _GLIBCXX_DEBUG=1
        _GLIBCXX_DEBUG_PEDANTIC=1
        _GLIBCXX_CONCEPT_CHECKS=1)
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        -UNDEBUG)
    set(TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        PARENT_SCOPE)
    set(TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        PARENT_SCOPE)
endfunction()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        configure_gcc_or_clang_gcc_options()
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "AppleClang")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    configure_gcc_or_clang_gcc_options()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

message(TEST_COMPILE_DEFINITIONS=${TEST_COMPILE_DEFINITIONS})
message(TEST_COMPILE_OPTIONS=${TEST_COMPILE_OPTIONS})

foreach(target_filename ${TestFilenames})
    string(CONCAT target_cpp_filename ${target_filename} ".cpp")
    string(CONCAT target_cxx_17 "target_cxx_17_" ${target_filename})
    string(CONCAT target_cxx_20 "target_cxx_20_" ${target_filename})

    add_executable(${target_cxx_17} ${target_cpp_filename})
    target_link_libraries(${target_cxx_17} gmp gmpxx mpfr)
    target_compile_options(${target_cxx_17} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target_cxx_17} PRIVATE ${TEST_COMPILE_DEFINITIONS})
    set_target_properties(${target_cxx_17} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${target_cxx_17} COMMAND $<TARGET_FILE:${target_cxx_17}>)

    add_executable(${target_cxx_20} ${target_cpp_filename})
    target_link_libraries(${target_cxx_20} gmp gmpxx mpfr)
    target_compile_options(${target_cxx_20} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target_cxx_20} PRIVATE ${TEST_COMPILE_DEFINITIONS})
    set_target_properties(${target_cxx_20} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${target_cxx_20} COMMAND $<TARGET_FILE:${target_cxx_20}>)

endforeach()

enable_testing()
