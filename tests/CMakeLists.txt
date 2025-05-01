cmake_minimum_required(VERSION 3.20)

project(tests VERSION 0.1.0 LANGUAGES C CXX)

include(CMakeTestsUtils.cmake)

set(TEST_COMPILE_DEFINITIONS
    ${TEST_COMPILE_DEFINITIONS}
    ENABLE_LONGINT_DEBUG_ASSERTS=1)
set(CAN_COMPILE_CONSTEXPR_STRING_MAP False)

set(TestFilenames "dummy")
set(TestDirectories "dummy")
set(TestLangVersions "dummy")
set(TestDependencies "dummy")
set(TestOptionalDependencies "dummy")
set(TestIsCProject "dummy")
set(TestCompileOnly "dummy")

if(NOT USING_MINGW_GCC_32)
    list(APPEND TestFilenames "test_fibonacci_num.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_gosper_algorithm.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_integers_128_bit.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_math_functions.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    if(LINUX AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"))
        list(APPEND TestOptionalDependencies "mpfr")
    else()
        list(APPEND TestOptionalDependencies "")
    endif()
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_is_prime_bpsw.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    if(LINUX AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"))
        list(APPEND TestOptionalDependencies "gmp")
    else()
        list(APPEND TestOptionalDependencies "")
    endif()
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_kronecker_symbol.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    if(LINUX AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU"))
        list(APPEND TestOptionalDependencies "gmp gmpxx")
    else()
        list(APPEND TestOptionalDependencies "")
    endif()
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_long_int.cpp")
    list(APPEND TestDirectories "number_theory")
    list(APPEND TestLangVersions "17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)
endif()

list(APPEND TestFilenames "test_bitmatrix.cpp")
list(APPEND TestDirectories "number_theory")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_cnk_counter.cpp")
list(APPEND TestDirectories "number_theory")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

set(mingw_gcc_or_clang USING_MINGW_GCC OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
if((NOT mingw_gcc_or_clang) OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 10.0.0))
    # gcc mingw w64 10.0.0 supports C++20 but doesn't have std::bit_cast, same with clang
    # gcc:   https://gcc.gnu.org/projects/cxx-status.html
    # clang: https://clang.llvm.org/cxx_status.html
    list(APPEND TestFilenames "test_hungarian_algo.cpp")
    list(APPEND TestDirectories "graphs HungarianAlgorithm")
    list(APPEND TestLangVersions "20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)
endif()
unset(mingw_gcc_or_clang)

list(APPEND TestFilenames "test_actrie.cpp")
list(APPEND TestDirectories "tf_idf_actrie")
list(APPEND TestLangVersions "20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_tf_idf_actrie.cpp search_lib.cpp")
list(APPEND TestDirectories "tf_idf_actrie")
list(APPEND TestLangVersions "20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU") AND NOT APPLE)
    list(APPEND TestFilenames "test_memcount.c")
    list(APPEND TestDirectories "vec_instructs")
    list(APPEND TestLangVersions "99 11 17 23")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject True)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_memcount.cpp")
    list(APPEND TestDirectories "vec_instructs")
    list(APPEND TestLangVersions "11 14 17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_memset_int.c")
    list(APPEND TestDirectories "vec_instructs")
    list(APPEND TestLangVersions "99 11 17 23")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject True)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "test_memset_int.cpp")
    list(APPEND TestDirectories "vec_instructs")
    list(APPEND TestLangVersions "11 14 17 20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0)
        set(CAN_COMPILE_CONSTEXPR_STRING_MAP True)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.4)
        set(CAN_COMPILE_CONSTEXPR_STRING_MAP True)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(MSVC_VERSION GREATER_EQUAL 1938)
        set(CAN_COMPILE_CONSTEXPR_STRING_MAP True)
    endif()
endif()

if(CAN_COMPILE_CONSTEXPR_STRING_MAP)
    list(APPEND TestFilenames "test_string_switch_map.cpp")
    list(APPEND TestDirectories "string-switch-map tests")
    list(APPEND TestLangVersions "20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly False)

    list(APPEND TestFilenames "usage_example.cpp")
    list(APPEND TestDirectories "string-switch-map")
    list(APPEND TestLangVersions "20 23 26")
    list(APPEND TestDependencies "")
    list(APPEND TestOptionalDependencies "")
    list(APPEND TestIsCProject False)
    list(APPEND TestCompileOnly True)
endif()
unset(CAN_COMPILE_CONSTEXPR_STRING_MAP)

if (GENERATOR_SUPPORTS_MODULES AND COMPILER_SUPPORTS_MODULES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../bstrees ${CMAKE_CURRENT_BINARY_DIR}/bstrees)
    add_test(NAME rbtree_tests COMMAND $<TARGET_FILE:rbtree_tests>)
endif()

list(APPEND TestFilenames "test_dsu.cpp")
list(APPEND TestDirectories "DisjointSetUnion")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_get_typename.cpp")
list(APPEND TestDirectories "misc tests")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_join_strings.cpp")
list(APPEND TestDirectories "misc tests")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_string_utils.cpp")
list(APPEND TestDirectories "misc tests")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_str_tools.cpp")
list(APPEND TestDirectories "str_functions")
list(APPEND TestLangVersions "17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_levenshtein_distance.cpp")
list(APPEND TestDirectories "str_functions")
list(APPEND TestLangVersions "20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(APPEND TestFilenames "test_enum_utils.cpp")
list(APPEND TestDirectories "misc tests")
list(APPEND TestLangVersions "11 14 17 20 23 26")
list(APPEND TestDependencies "")
list(APPEND TestOptionalDependencies "")
list(APPEND TestIsCProject False)
list(APPEND TestCompileOnly False)

list(POP_FRONT TestFilenames) # pop dummy
list(POP_FRONT TestDirectories) # pop dummy
list(POP_FRONT TestLangVersions) # pop dummy
list(POP_FRONT TestDependencies) # pop dummy
list(POP_FRONT TestOptionalDependencies) # pop dummy
list(POP_FRONT TestIsCProject) # pop dummy
list(POP_FRONT TestCompileOnly) # pop dummy


function(
    add_test_target
    target_filenames
    target_dir
    target_dependencies
    target_optional_dependencies
    target_lang_version
    is_c_project
    compile_only
)
    string(REPLACE " " ";" target_filenames "${target_filenames}")

    set(target_full_filenames)
    string(REPLACE " " ";" target_dir "${target_dir}")
    foreach(target_filename ${target_filenames})
        string(JOIN "/" target_path_to_cpp ".." ${target_dir} ${target_filename})
        list(APPEND target_full_filenames ${target_path_to_cpp})
    endforeach()

    list(GET target_filenames 0 first_target_filename)
    # remove file extension
    string(REGEX REPLACE "\\.[^.]*$" "" first_target_filename ${first_target_filename})
    if(is_c_project)
        set(target "target_c_${target_lang_version}_${first_target_filename}")
    else()
        set(target "target_cxx_${target_lang_version}_${first_target_filename}")
    endif()

    string(COMPARE EQUAL "${target_dependencies}" "" empty_dependencies)
    if(NOT empty_dependencies)
        string(REPLACE " " ";" target_dependencies "${target_dependencies}")
    endif()

    string(COMPARE EQUAL "${target_optional_dependencies}" "" empty_optional_dependencies)
    if(NOT empty_optional_dependencies)
        string(REPLACE " " ";" target_optional_dependencies "${target_optional_dependencies}")
    endif()

    message(STATUS "+-")
    message(STATUS "| target = ${target}")
    message(STATUS "| target_full_filenames = ${target_full_filenames}")
    message(STATUS "| target_dependencies = ${target_dependencies}")
    message(STATUS "| target_optional_dependencies = ${target_optional_dependencies}")
    message(STATUS "| target_lang_version = ${target_lang_version}")
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

    if(has_missing_dependency)
        return()
    endif()

    add_executable(${target} ${target_full_filenames})

    if(NOT empty_dependencies)
        target_link_libraries(${target} PUBLIC ${target_dependencies})
    endif()

    if(NOT empty_optional_dependencies)
        foreach(target_optional_dependency ${target_optional_dependencies})
            find_library(dependency_lib ${target_optional_dependency})
            if(NOT dependency_lib)
                message(WARNING "Optional dependency ${target_optional_dependency} of target ${target} not found")
            else()
                target_link_libraries(${target} PUBLIC ${target_optional_dependency})
            endif()
        endforeach()
    endif()

    if(LINK_THREADS_LIBRARY_MANUALLY)
        target_link_libraries(${target} PUBLIC Threads::Threads)
    endif()

    if(LINK_COMPILER_RT_BUILTINS_MANUALLY)
        target_link_libraries(${target} PUBLIC clang_rt.builtins-x86_64)
    endif()

    if(USING_MINGW_GCC AND NOT WIN32)
        target_link_options(${target} PUBLIC -static -static-libgcc -static-libstdc++)
    endif()

    target_compile_definitions(${target} PUBLIC ${TEST_COMPILE_DEFINITIONS})
    if(is_c_project)
        target_compile_options(${target} PUBLIC ${TEST_C_COMPILE_OPTIONS})
        set_target_properties(${target} PROPERTIES
            C_STANDARD ${target_lang_version}
            C_EXTENSIONS OFF
            C_STANDARD_REQUIRED ON
            LINKER_LANGUAGE C)
    else()
        target_compile_options(${target} PUBLIC ${TEST_CXX_COMPILE_OPTIONS})
        set_target_properties(${target} PROPERTIES
            CXX_STANDARD ${target_lang_version}
            CXX_EXTENSIONS OFF
            CXX_STANDARD_REQUIRED ON)
    endif()
    if(NOT compile_only)
        add_test(NAME ${target} COMMAND $<TARGET_FILE:${target}>)
    endif()
endfunction(add_test_target)

foreach(target_filenames
        target_dir
        target_supported_lang_versions
        target_dependencies
        target_optional_dependencies
        is_c_project
        compile_only
    IN ZIP_LISTS
        TestFilenames
        TestDirectories
        TestLangVersions
        TestDependencies
        TestOptionalDependencies
        TestIsCProject
        TestCompileOnly)
    string(REPLACE " " ";" target_supported_lang_versions "${target_supported_lang_versions}")
    foreach(version ${target_supported_lang_versions})
        if(
            (NOT is_c_project AND ${version} IN_LIST COMPILER_SUPPORTED_CXX_VERSIONS)
            OR
            (is_c_project AND ${version} IN_LIST COMPILER_SUPPORTED_C_VERSIONS)
        )
            add_test_target(
                "${target_filenames}"
                "${target_dir}"
                "${target_dependencies}"
                "${target_optional_dependencies}"
                ${version}
                ${is_c_project}
                ${compile_only}
            )
        endif()
    endforeach()
endforeach()

enable_testing()
