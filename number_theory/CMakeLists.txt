cmake_minimum_required(VERSION 3.5)

project(math_tests VERSION 0.1.0 LANGUAGES CXX)

set(USING_MINGW_GCC False)
set(USING_MINGW_GCC_32 False)
set(USING_MSYS2_MINGW_GCC False)

if (MINGW OR MSYS)
    set(USING_MINGW_GCC True)
    if((WIN32 AND NOT MSVC) OR MSYS)
        set(USING_MSYS2_MINGW_GCC True)
    endif()
else()
    string(TOLOWER ${CMAKE_CXX_COMPILER} STRING_LOWER_CMAKE_CXX_COMPILER)
    string(FIND ${STRING_LOWER_CMAKE_CXX_COMPILER} "mingw" pos)
    if (NOT pos EQUAL -1)
        set(USING_MINGW_GCC True)
    endif()
endif()

if (USING_MINGW_GCC)
    string(FIND ${STRING_LOWER_CMAKE_CXX_COMPILER} "i686" pos)
    if (NOT pos EQUAL -1)
        set(USING_MINGW_GCC_32 True)
    endif()
endif()

set(TestFilenames)
set(TestDependencies "dummy")
set(TestOptionalDependencies "dummy")

list(APPEND TestFilenames test_fibonacci_num)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_gosper_algorithm)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_integers_128_bit)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_is_prime_bpsw)
list(APPEND TestDependencies "gmp")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_kronecker_symbol)
list(APPEND TestDependencies "gmp gmpxx")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_long_int)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames test_math_functions)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "mpfr")

list(APPEND TestFilenames test_bitmatrix)
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(POP_FRONT TestDependencies)
list(POP_FRONT TestOptionalDependencies)

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
        -Wformat=2
    )
    if (NOT USING_MINGW_GCC)
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            -fstack-protector-strong
            -mshstk
        )
    endif()
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

    if (NOT USING_MINGW_GCC)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=address PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=undefined PARENT_SCOPE)
    endif()

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

message(STATUS "|- TEST_COMPILE_DEFINITIONS = ${TEST_COMPILE_DEFINITIONS}")
message(STATUS "|- TEST_COMPILE_OPTIONS = ${TEST_COMPILE_OPTIONS}")
message(STATUS "|- USING_MINGW_GCC = ${USING_MINGW_GCC}")
message(STATUS "|- USING_MINGW_GCC_32 = ${USING_MINGW_GCC_32}")
message(STATUS "|- USING_MSYS2_MINGW_GCC = ${USING_MSYS2_MINGW_GCC}")

foreach(target_filename target_dependencies target_optional_dependencies IN ZIP_LISTS TestFilenames TestDependencies TestOptionalDependencies)
    string(COMPARE EQUAL "${target_dependencies}" "" empty_dependencies)
    if (NOT empty_dependencies)
        string(REPLACE " " ";" target_dependencies ${target_dependencies})
    endif()

    string(COMPARE EQUAL "${target_optional_dependencies}" "" empty_optional_dependencies)
    if (NOT empty_optional_dependencies)
        string(REPLACE " " ";" target_optional_dependencies ${target_optional_dependencies})
    endif()

    message(STATUS "target_filename = ${target_filename}, target_dependencies = ${target_dependencies}")

    set(has_missing_dependency FALSE)
    foreach(target_dependency ${target_dependencies})
        find_library(dependency_lib ${target_dependency})
        if(NOT dependency_lib)
            message(WARNING "Required dependency ${target_dependency} of target ${target_filename} not found, skipping target")
            set(has_missing_dependency TRUE)
            break()
        endif()
    endforeach()

    if (has_missing_dependency)
        continue()
    endif()

    string(CONCAT target_cpp_filename ${target_filename} ".cpp")
    string(CONCAT target_cxx_17 "target_cxx_17_" ${target_filename})
    string(CONCAT target_cxx_20 "target_cxx_20_" ${target_filename})

    add_executable(${target_cxx_17} ${target_cpp_filename})

    if (NOT empty_dependencies)
        target_link_libraries(${target_cxx_17} ${target_dependencies})
    endif()

    if (NOT empty_optional_dependencies)
        foreach(target_optional_dependency ${target_optional_dependencies})
            find_library(dependency_lib ${target_optional_dependency})
            if(NOT dependency_lib)
                message(WARNING "Optional dependency ${target_optional_dependency} of target ${target_filename} not found")
            else()
                target_link_libraries(${target_cxx_17} ${target_optional_dependency})
            endif()
        endforeach()
    endif()

    if (USING_MINGW_GCC AND NOT WIN32)
        target_link_options(${target_cxx_17} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    target_compile_options(${target_cxx_17} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target_cxx_17} PRIVATE ${TEST_COMPILE_DEFINITIONS})
    set_target_properties(${target_cxx_17} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${target_cxx_17} COMMAND $<TARGET_FILE:${target_cxx_17}>)

    add_executable(${target_cxx_20} ${target_cpp_filename})

    if (NOT empty_dependencies)
        target_link_libraries(${target_cxx_20} ${target_dependencies})
    endif()

    if (NOT empty_optional_dependencies)
        foreach(target_optional_dependency ${target_optional_dependencies})
            find_library(dependency_lib ${target_optional_dependency})
            if(NOT dependency_lib)
                message(WARNING "Optional dependency ${target_optional_dependency} of target ${target_filename} not found")
            else()
                target_link_libraries(${target_cxx_20} ${target_optional_dependency})
            endif()
        endforeach()
    endif()

    if (USING_MINGW_GCC AND NOT WIN32)
        target_link_options(${target_cxx_20} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    target_compile_options(${target_cxx_20} PRIVATE ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target_cxx_20} PRIVATE ${TEST_COMPILE_DEFINITIONS})
    set_target_properties(${target_cxx_20} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${target_cxx_20} COMMAND $<TARGET_FILE:${target_cxx_20}>)
endforeach()

enable_testing()
