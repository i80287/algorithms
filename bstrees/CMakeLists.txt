cmake_minimum_required(VERSION 3.28)
project(tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
# Empty by default
set(TEST_C_COMPILE_OPTIONS)
set(TEST_CXX_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

function(configure_gcc_or_clang_gcc_options)
    if (MSVC)
        message(FATAL_ERROR "ClangCL should not be configured with this function")
    endif ()

    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${TEST_C_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${TEST_CXX_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${TEST_COMPILE_DEFINITIONS})
    set(LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO
            ${CMAKE_C_FLAGS_RELWITHDEBINFO})
    set(LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO
            ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

    # Flags for both C and C++
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Wall
            -Wextra
            -Wcast-qual
            -Wpedantic
            -Wunused
            -Wshadow
            -Wnull-dereference
            -Wundef
            -Wsign-conversion
            -Wsign-compare
            -Wconversion
            -Wmissing-noreturn
            -Wunreachable-code
            -Wcast-align
            -Wformat=2
            -Wswitch-bool
            -Wswitch-default
            -Wswitch-enum
            -Wdeprecated
            -Wtype-limits
            -Werror
            -pedantic-errors
    )
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Warray-bounds=2
                -Wshift-overflow=2
                -Wlogical-op
                -Wunsafe-loop-optimizations
                -Wduplicated-cond
                -fdiagnostics-color=always)
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Warray-bounds
                -Wshift-overflow
                -Wshift-sign-overflow
                -Wshorten-64-to-32
                -Wthread-safety
                -Wnull-pointer-arithmetic
                -Wnull-pointer-subtraction
                -Wnullable-to-nonnull-conversion
                -fcolor-diagnostics
                -fansi-escape-codes
                -ftemplate-backtrace-limit=0)
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
                ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
                _LIBCPP_ENABLE_ASSERTIONS)
    endif ()
    if (NOT USING_MINGW_GCC AND CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -fstack-protector-strong
        )
    endif ()

    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
    )

    # Flags for C only
    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            -Wwrite-strings
            -Wbad-function-cast
            -Wint-conversion
    )
    # Flags for C++ only
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Weffc++
            -Wsign-promo
            -Wold-style-cast
    )
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -U_GLIBCXX_USE_DEPRECATED)
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _GLIBCXX_SANITIZE_VECTOR
            ENABLE_LONGINT_DEBUG_ASSERTS=1)
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0)
        # In gcc with version < 10.0 these checks break `constexpr`-tivity of some std:: functions
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
                ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
                _GLIBCXX_DEBUG
                _GLIBCXX_DEBUG_PEDANTIC
                _GLIBCXX_CONCEPT_CHECKS)
    endif ()

    # Remove -DNDEBUG from the RelWithDebInfo mode flags
    string(REGEX REPLACE "-D\\s?NDEBUG(=\\d)?" "" LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO "${LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO}")
    string(REGEX REPLACE "-D\\s?NDEBUG(=\\d)?" "" LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO "${LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

    set(TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            PARENT_SCOPE)
    set(TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            PARENT_SCOPE)
    set(TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            PARENT_SCOPE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO
            ${LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO}
            PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
            ${LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO}
            PARENT_SCOPE)

    if (NOT USING_MINGW_GCC)
        # set(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} -fsanitize=address,undefined PARENT_SCOPE)
        # set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=address,undefined PARENT_SCOPE)
    endif ()

endfunction(configure_gcc_or_clang_gcc_options)

function(configure_msvc_or_clang_msvc_options)
    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${TEST_C_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${TEST_CXX_COMPILE_OPTIONS})
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${TEST_COMPILE_DEFINITIONS})

    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _CRT_SECURE_NO_WARNINGS=1)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -UNDEBUG)
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                "/W4" # Enable -Wall and -Wextra
        )
    else ()
        if (MSVC_VERSION GREATER_EQUAL 1914)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                    ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                    "/Zc:__cplusplus")
        endif ()
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                "/W4"
                "/UNDEBUG"
                "/wd4146" # Disable C4146: unary minus operator applied to unsigned type, result still unsigned
        )
    endif ()

    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS})

    set(TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            PARENT_SCOPE)
    set(TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
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
    else ()
        message(FATAL_ERROR "Clang with unknown frontend")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    configure_gcc_or_clang_gcc_options()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    configure_msvc_or_clang_msvc_options()
endif ()

add_executable(tests RBTreeTests.cpp)
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=address,undefined,leak,bounds)

target_compile_definitions(tests PRIVATE ${COMPILE_DEFINITIONS})
target_compile_options(tests PRIVATE ${TEST_CXX_COMPILE_OPTIONS})

add_library(rbtree)
target_sources(rbtree PUBLIC
        FILE_SET CXX_MODULES
        FILES RBTree.cppm)

target_compile_definitions(rbtree PRIVATE ${COMPILE_DEFINITIONS} RBTREE_DEBUG)
target_compile_options(rbtree PRIVATE ${TEST_CXX_COMPILE_OPTIONS})

target_link_libraries(tests PRIVATE rbtree)
