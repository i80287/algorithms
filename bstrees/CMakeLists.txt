cmake_minimum_required(VERSION 3.28)
project(tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMPILE_OPTIONS)
set(COMPILE_DEFINITIONS)

function(configure_gcc_compile_options)
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
    endif ()
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
                ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
                _LIBCPP_ENABLE_ASSERTIONS=1)
    endif ()
    set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _GLIBCXX_DEBUG=1
            _GLIBCXX_DEBUG_PEDANTIC=1
            _GLIBCXX_CONCEPT_CHECKS=1
            #            _GLIBCXX_DEBUG_BACKTRACE=1
    )
    set(LOCAL_FN_TEST_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            -UNDEBUG)
    set(COMPILE_OPTIONS
            ${LOCAL_FN_TEST_COMPILE_OPTIONS}
            PARENT_SCOPE)
    set(COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            PARENT_SCOPE)
    # target_link_options(tgself PUBLIC "LINKER:-lstdc++exp,-lstdc++_libbacktrace")
endfunction()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        configure_gcc_compile_options()
    elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "AppleClang")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    configure_gcc_compile_options()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif ()

add_executable(tests RBTreeTests.cpp)
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=address,undefined,leak,bounds)

target_compile_definitions(tests PRIVATE ${COMPILE_DEFINITIONS})
target_compile_options(tests PRIVATE ${COMPILE_OPTIONS})

add_library(rbtree)
target_sources(rbtree PUBLIC
        FILE_SET CXX_MODULES
        FILES RBTree.cppm)

target_compile_definitions(rbtree PRIVATE ${COMPILE_DEFINITIONS} RBTREE_DEBUG)
target_compile_options(rbtree PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(tests PRIVATE rbtree)
