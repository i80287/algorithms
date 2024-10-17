cmake_minimum_required(VERSION 3.20)

project(tests VERSION 0.1.0 LANGUAGES CXX)

set(USING_MINGW_GCC False)
set(USING_MINGW_GCC_32 False)
set(USING_MSYS2_MINGW_GCC False)

# other versions can be added below depending on the compiler version
set(SUPPORTED_CXX_VERSIONS "17")

string(TOLOWER ${CMAKE_CXX_COMPILER} STRING_LOWER_CMAKE_CXX_COMPILER)
if (MINGW OR MSYS)
    set(USING_MINGW_GCC True)
    if((WIN32 AND NOT MSVC) OR MSYS)
        set(USING_MSYS2_MINGW_GCC True)
    endif()
else()
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

set(LINK_THREADS_LIBRARY_MANUALLY False)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_program(LSB_RELEASE_EXEC lsb_release)
    if (NOT LSB_RELEASE_EXEC)
        message(WARNING "Could not get linux distro info, pthread linking on ubuntu 20 with g++ might be broken")
    else()
        execute_process(COMMAND ${LSB_RELEASE_EXEC} -irs
            OUTPUT_VARIABLE
            LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        string(REPLACE "\n" " " LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT "${LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT}")
        separate_arguments(LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT)

        list(GET LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT 0 LSB_DISTRIBUTOR_ID_SHORT)
        list(GET LSB_DISTRIBUTOR_ID_AND_RELEASE_VERSION_SHORT 1 LSB_RELEASE_VERSION_SHORT)

        string(TOLOWER "${LSB_DISTRIBUTOR_ID_SHORT}" LSB_DISTRIBUTOR_ID_SHORT)
        if (LSB_DISTRIBUTOR_ID_SHORT STREQUAL "ubuntu" AND LSB_RELEASE_VERSION_SHORT VERSION_LESS_EQUAL "20.04")
            set(LINK_THREADS_LIBRARY_MANUALLY True)
            set(THREADS_PREFER_PTHREAD_FLAG ON)
            find_package(Threads REQUIRED)
        endif()
    endif()
endif()

# Empty by default
set(TEST_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

function(configure_gcc_or_clang_gcc_options)
    if (MSVC)
        message(FATAL_ERROR "ClangCL should not be configured with this function")
    endif()

    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${TEST_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${TEST_COMPILE_DEFINITIONS})
    set(LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO
        ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        -Wall
        -Wextra
        -Wcast-qual
        -Wpedantic
        -Wshift-overflow
        -Wunused
        -Wshadow
        -Wnull-dereference
        -Wundef
        -Wwrite-strings
        -Wsign-conversion
        -Wsign-compare
        -Wsign-promo
        -Wconversion
        -Wmissing-noreturn
        -Wunreachable-code
        -Wcast-align
        -Warray-bounds
        -Wformat=2
        -Wswitch-bool
        -Wswitch-default
        -Wswitch-enum
        -Wold-style-cast
        -Werror
        -pedantic-errors
    )
    if (NOT USING_MINGW_GCC AND CMAKE_SYSTEM_NAME MATCHES "Linux")
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
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            -Wshift-sign-overflow
            -Wshorten-64-to-32
            -Wdeprecated-dynamic-exception-spec
            -Wthread-safety
            -Wtype-limits
            -Wzero-as-null-pointer-constant)
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _LIBCPP_ENABLE_ASSERTIONS)
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0)
        # In gcc with version < 10.0 these checks break `constexpr`-tivity of some std:: functions
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _GLIBCXX_DEBUG
            _GLIBCXX_DEBUG_PEDANTIC
            _GLIBCXX_CONCEPT_CHECKS)
    endif()
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        _GLIBCXX_SANITIZE_VECTOR)
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        -U_GLIBCXX_USE_DEPRECATED)

    string(REGEX REPLACE "-D\\s?NDEBUG(=\\d)?" "" LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO "${LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    set(TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        PARENT_SCOPE)
    set(TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
        ${LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO}
        PARENT_SCOPE)

    if (NOT USING_MINGW_GCC)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=address,undefined PARENT_SCOPE)
    endif()

endfunction(configure_gcc_or_clang_gcc_options)

function(configure_msvc_or_clang_msvc_options)
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
        ${TEST_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${TEST_COMPILE_DEFINITIONS})

    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        _CRT_SECURE_NO_WARNINGS=1)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            -UNDEBUG)
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            "/W4" # Enable -Wall and -Wextra
        )
    else()
        if (MSVC_VERSION GREATER_EQUAL 1914)
            set(LOCAL_FN_TEST_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_COMPILE_OPTIONS}
                "/Zc:__cplusplus")
        endif()
        set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            "/W4"
            "/UNDEBUG"
            "/wd4146" # Disable C4146: unary minus operator applied to unsigned type, result still unsigned
            )
    endif()

    set(TEST_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_COMPILE_OPTIONS}
        PARENT_SCOPE)
    set(TEST_COMPILE_DEFINITIONS
        ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
        PARENT_SCOPE)
endfunction(configure_msvc_or_clang_msvc_options)


# https://en.cppreference.com/w/cpp/compiler_support
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        configure_msvc_or_clang_msvc_options()
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        configure_gcc_or_clang_gcc_options()
    else()
        message(FATAL_ERROR "Clang with unknown frontend")
    endif()

    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 20)
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 23)
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 26)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    configure_gcc_or_clang_gcc_options()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 20)
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 23)
    endif()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
        list(APPEND SUPPORTED_CXX_VERSIONS 26)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    configure_msvc_or_clang_msvc_options()
    if (MSVC_VERSION GREATER_EQUAL 1928)
        list(APPEND SUPPORTED_CXX_VERSIONS 20)
    endif()
    if (MSVC_VERSION GREATER_EQUAL 1937)
        list(APPEND SUPPORTED_CXX_VERSIONS 23)
    endif()
endif()


function(manually_add_byte_order_to_cppcheck_arguments)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if (CMAKE_CXX_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
            list(APPEND CMAKE_CXX_CPPCHECK
                "-D__ORDER_LITTLE_ENDIAN__=1234"
                "-D__ORDER_BIG_ENDIAN__=4321"
                "-D__BYTE_ORDER__=1234"
            )
        elseif (CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
            list(APPEND CMAKE_CXX_CPPCHECK
                "-D__ORDER_LITTLE_ENDIAN__=1234"
                "-D__ORDER_BIG_ENDIAN__=4321"
                "-D__BYTE_ORDER__=4321"
            )
        endif()
    endif()
endfunction()


if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    if (NOT CMAKE_CXX_CPPCHECK)
        message(WARNING "Could not find cppcheck")
    else()
        list(APPEND
            CMAKE_CXX_CPPCHECK
            "--force"
            "--template=gcc"
            "--error-exitcode=0"
            "--inline-suppr"
        )
    endif()
endif()

message(STATUS "+-")
message(STATUS "| TEST_COMPILE_DEFINITIONS = ${TEST_COMPILE_DEFINITIONS}")
message(STATUS "| TEST_COMPILE_OPTIONS = ${TEST_COMPILE_OPTIONS}")
message(STATUS "| USING_MINGW_GCC = ${USING_MINGW_GCC}")
message(STATUS "| USING_MINGW_GCC_32 = ${USING_MINGW_GCC_32}")
message(STATUS "| USING_MSYS2_MINGW_GCC = ${USING_MSYS2_MINGW_GCC}")
message(STATUS "+-")


set(TestFilenames "dummy")
set(TestDirectories "dummy")
set(TestCxxVersions "dummy")
set(TestDependencies "dummy")
set(TestOptionalDependencies "dummy")

if (NOT USING_MINGW_GCC_32)
    list(APPEND TestFilenames "test_fibonacci_num")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestCxxVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")

    list(APPEND TestFilenames "test_gosper_algorithm")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestCxxVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")

    if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
        list(APPEND TestFilenames "test_integers_128_bit")
        list(APPEND TestDirectories "number_theory")
        list(APPEND TestCxxVersions "17 20 23 26")
        list(APPEND TestDependencies "")
        list(APPEND TestOptionalDependencies "")

        list(APPEND TestFilenames "test_math_functions")
        list(APPEND TestDirectories "number_theory")
        list(APPEND TestCxxVersions "17 20 23 26")
        list(APPEND TestDependencies "")
        if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU") AND NOT APPLE)
            list(APPEND TestOptionalDependencies "mpfr")
        else()
            list(APPEND TestOptionalDependencies "")
        endif()
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        list(APPEND TestFilenames "test_is_prime_bpsw")
        list(APPEND TestDirectories "number_theory")
        list(APPEND TestCxxVersions "17 20 23 26")
        list(APPEND TestDependencies "")
        if (NOT APPLE)
            list(APPEND TestOptionalDependencies "gmp")
        else()
            list(APPEND TestOptionalDependencies "")
        endif()

        list(APPEND TestFilenames "test_kronecker_symbol")
        list(APPEND TestDirectories "number_theory")
        list(APPEND TestCxxVersions "17 20 23 26")
        list(APPEND TestDependencies "")
        if (NOT APPLE)
            list(APPEND TestOptionalDependencies "gmp gmpxx")
        else()
            list(APPEND TestOptionalDependencies "")
        endif()

        list(APPEND TestFilenames "test_long_int")
        list(APPEND TestDirectories "number_theory")
        list(APPEND TestCxxVersions "17 20 23 26")
        list(APPEND TestDependencies "")
        list(APPEND TestOptionalDependencies "")
    endif()
endif()

list(APPEND TestFilenames "test_bitmatrix")
list(APPEND TestDirectories "number_theory")
list(APPEND TestCxxVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames "test_cnk_counter")
list(APPEND TestDirectories "number_theory")
list(APPEND TestCxxVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

set(mingw_gcc_or_clang USING_MINGW_GCC OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
if ((NOT mingw_gcc_or_clang) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10.0.0))
    # gcc mingw w64 10.0.0 supports C++20 but doesn't have std::bit_cast, same with clang
    # gcc:   https://gcc.gnu.org/projects/cxx-status.html
    # clang: https://clang.llvm.org/cxx_status.html
    list(APPEND TestFilenames "test_hungarian_algo")
    list(APPEND TestDirectories "graphs HungarianAlgorithm")
    list(APPEND TestCxxVersions "20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
endif()
unset(mingw_gcc_or_clang)

list(APPEND TestFilenames "test_actrie")
list(APPEND TestDirectories "tf_idf_actrie")
list(APPEND TestCxxVersions "20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(APPEND TestFilenames "test_tf_idf_actrie search_lib")
list(APPEND TestDirectories "tf_idf_actrie")
list(APPEND TestCxxVersions "20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")

list(POP_FRONT TestFilenames)
list(POP_FRONT TestDirectories)
list(POP_FRONT TestCxxVersions)
list(POP_FRONT TestDependencies)
list(POP_FRONT TestOptionalDependencies)


function(
    add_test_target
    target_filenames
    target_dir
    target_dependencies
    target_optional_dependencies
    target_cxx_version
)
    string(REPLACE " " ";" target_filenames "${target_filenames}")

    set(target_cpp_filenames)
    string(REPLACE " " ";" target_dir "${target_dir}")
    foreach(target_filename ${target_filenames})
        set(target_cpp_filename "${target_filename}.cpp")
        string(JOIN "/" target_path_to_cpp ".." ${target_dir} ${target_cpp_filename})
        list(APPEND target_cpp_filenames ${target_path_to_cpp})
    endforeach()

    list(GET target_filenames 0 first_target_filename)
    set(target "target_cxx_${target_cxx_version}_${first_target_filename}")

    string(COMPARE EQUAL "${target_dependencies}" "" empty_dependencies)
    if (NOT empty_dependencies)
        string(REPLACE " " ";" target_dependencies "${target_dependencies}")
    endif()

    string(COMPARE EQUAL "${target_optional_dependencies}" "" empty_optional_dependencies)
    if (NOT empty_optional_dependencies)
        string(REPLACE " " ";" target_optional_dependencies "${target_optional_dependencies}")
    endif()

    message(STATUS "+-")
    message(STATUS "| target = ${target}")
    message(STATUS "| target_cpp_filenames = ${target_cpp_filenames}")
    message(STATUS "| target_dependencies = ${target_dependencies}")
    message(STATUS "| target_optional_dependencies = ${target_optional_dependencies}")
    message(STATUS "| target_cxx_version = ${target_cxx_version}")
    message(STATUS "+-")

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
        return()
    endif()

    add_executable(${target} ${target_cpp_filenames})

    if (NOT empty_dependencies)
        target_link_libraries(${target} PUBLIC ${target_dependencies})
    endif()

    if (NOT empty_optional_dependencies)
        foreach(target_optional_dependency ${target_optional_dependencies})
            find_library(dependency_lib ${target_optional_dependency})
            if(NOT dependency_lib)
                message(WARNING "Optional dependency ${target_optional_dependency} of target ${target} not found")
            else()
                target_link_libraries(${target} PUBLIC ${target_optional_dependency})
            endif()
        endforeach()
    endif()

    if (LINK_THREADS_LIBRARY_MANUALLY)
        target_link_libraries(${target} PUBLIC Threads::Threads)
    endif()

    if (USING_MINGW_GCC AND NOT WIN32)
        target_link_options(${target} PUBLIC -static -static-libgcc -static-libstdc++)
    endif()

    target_compile_options(${target} PUBLIC ${TEST_COMPILE_OPTIONS})
    target_compile_definitions(${target} PUBLIC ${TEST_COMPILE_DEFINITIONS})
    set_target_properties(${target} PROPERTIES CXX_STANDARD ${target_cxx_version} CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)
    add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
endfunction(add_test_target)

foreach(target_filenames target_dir target_supported_cxx_versions target_dependencies target_optional_dependencies
    IN ZIP_LISTS
        TestFilenames
        TestDirectories
        TestCxxVersions
        TestDependencies
        TestOptionalDependencies)
    string(REPLACE " " ";" target_supported_cxx_versions "${target_supported_cxx_versions}")
    foreach(version ${SUPPORTED_CXX_VERSIONS})
        if (${version} IN_LIST target_supported_cxx_versions)
            add_test_target(
                "${target_filenames}"
                "${target_dir}"
                "${target_dependencies}"
                "${target_optional_dependencies}"
                ${version}
            )
        endif()
    endforeach()
endforeach()

enable_testing()
