cmake_minimum_required(VERSION 3.28)
project(rbtree_datastructure)

include(../tests/CMakeTestsUtils.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CXX_STANDARD_REQUIRED ON)

add_library(rbtree STATIC)
target_sources(rbtree PUBLIC
        FILE_SET CXX_MODULES
        FILES RBTree.cppm)
target_compile_definitions(rbtree PRIVATE ${TEST_COMPILE_DEFINITIONS} RBTREE_DEBUG)
target_compile_options(rbtree PRIVATE ${TEST_CXX_COMPILE_OPTIONS})

add_executable(rbtree_tests RBTreeTests.cpp)
target_compile_definitions(rbtree_tests PRIVATE ${TEST_COMPILE_DEFINITIONS})
target_compile_options(rbtree_tests PRIVATE ${TEST_CXX_COMPILE_OPTIONS})
target_link_libraries(rbtree_tests PUBLIC rbtree)
