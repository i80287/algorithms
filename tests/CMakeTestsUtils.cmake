include_guard(GLOBAL)

set(USING_MINGW_GCC False)
set(USING_MINGW_GCC_32 False)
set(USING_MSYS2_MINGW_GCC False)

set(LINK_THREADS_LIBRARY_MANUALLY False)
set(LINK_COMPILER_RT_BUILTINS_MANUALLY False)

# Empty by default
set(TEST_C_COMPILE_OPTIONS)
set(TEST_CXX_COMPILE_OPTIONS)
set(TEST_COMPILE_DEFINITIONS)

set(USE_CPPCHECK_DURING_BUILD False)
set(USE_CLANG_TIDY_DURING_BUILD False)

set(CXX_MODULES_SUPPORTED False)
set(CMAKE_SUPPORTS_MODULES False)
set(GENERATOR_SUPPORTS_MODULES False)
set(COMPILER_SUPPORTS_MODULES False)

# Other versions can be added below depending on the compiler version
set(COMPILER_SUPPORTED_C_VERSIONS "99")
set(COMPILER_SUPPORTED_CXX_VERSIONS "11;14;17")

if (NOT DEFINED UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS OR NOT UTILS_FLAGS_FORCE_DISABLE_RUNTIME_CHECKS)
    set(ENABLE_RUNTIME_CHECKS_FLAG__ ON)
else()
    set(ENABLE_RUNTIME_CHECKS_FLAG__ OFF)
endif()

string(TOLOWER ${CMAKE_CXX_COMPILER} STRING_LOWER_CMAKE_CXX_COMPILER)
if(MINGW OR MSYS)
    set(USING_MINGW_GCC True)
    if((WIN32 AND NOT MSVC) OR MSYS)
        set(USING_MSYS2_MINGW_GCC True)
    endif()
else()
    string(FIND ${STRING_LOWER_CMAKE_CXX_COMPILER} "mingw" pos)
    if(NOT pos EQUAL -1)
        set(USING_MINGW_GCC True)
    endif()
endif()

if(USING_MINGW_GCC)
    string(FIND ${STRING_LOWER_CMAKE_CXX_COMPILER} "i686" pos)
    if(NOT pos EQUAL -1)
        set(USING_MINGW_GCC_32 True)
    endif()
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.28")
    set(CMAKE_SUPPORTS_MODULES True)
endif()

if(CMAKE_GENERATOR STREQUAL "Ninja" OR CMAKE_GENERATOR STREQUAL "Ninja Multi-Config" OR CMAKE_GENERATOR STREQUAL "Visual Studio 17 2022")
    set(GENERATOR_SUPPORTS_MODULES True)
endif()

function(configure_gcc_or_clang_gcc_options)
    if(MSVC)
        message(FATAL_ERROR "ClangCL should not be configured with this function")
    endif()

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

    cmake_host_system_information(RESULT phys_mem_in_mb QUERY TOTAL_PHYSICAL_MEMORY)
    math(EXPR allocation_limit_in_bytes "(${phys_mem_in_mb} * 1024 * 1024) * 8 / 10")
    set(frame_allocation_limit_in_bytes ${allocation_limit_in_bytes})
    # clang 18.1.3 's default value for max stack frame size is 4294967295 (uint32_t max value)
    # gcc 13.2.0 's default value for max object size is PTRDIFF_MAX (e.g. 9223372036854775807 on x86-64)
    if(frame_allocation_limit_in_bytes GREATER 268435456) # 256 mb
        set(frame_allocation_limit_in_bytes 268435456)
    endif()
    set(stack_allocation_limit_in_bytes ${frame_allocation_limit_in_bytes})

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
        -Wshift-negative-value
        -Walloca
        -Wdate-time
        -Wdouble-promotion
        -Wfloat-equal
        -Wunused-macros
        -Wframe-larger-than=${frame_allocation_limit_in_bytes}
        -Werror
        -pedantic-errors
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Warray-bounds=2
            -Wshift-overflow=2
            -Wlogical-op
            -Wunsafe-loop-optimizations
            -Wduplicated-cond
            -Wstrict-overflow=1
            -Wformat-overflow=2
            -Wsuggest-final-types
            -Wsuggest-attribute=const
            -Wsuggest-attribute=noreturn
            -Wsuggest-attribute=format
            -Wsuggest-attribute=cold
            -Wsuggest-attribute=malloc
            -Wstack-usage=${stack_allocation_limit_in_bytes}
            -Wattribute-alias=2
            -Wstringop-overflow=4
            -Wduplicated-branches
            -Walloc-zero
            -Wtrampolines
            -Wcast-align=strict
            -fdiagnostics-color=always
        )
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 15.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Walloc-size-larger-than=${allocation_limit_in_bytes}
                -Wlarger-than=${allocation_limit_in_bytes}
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Warith-conversion
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wenum-conversion # enabled by the -Wconversion if Clang is used or by the gcc in C mode (-Werror)
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Winvalid-utf8 # enabled by default by Clang but Clang only warns about invalid utf in the comments
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wflex-array-member-not-at-end
            )
        endif()
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Warray-bounds
            -Warray-bounds-pointer-arithmetic
            -Wshift-overflow
            -Wshift-sign-overflow
            -Wshorten-64-to-32
            -Wthread-safety
            -Wnullable-to-nonnull-conversion
            -Wstatic-in-inline
            -Wformat-pedantic
            -Wformat-type-confusion
            -Wfour-char-constants
            -Wgcc-compat
            -Wgnu
            -Widiomatic-parentheses
            -Wimplicit
            -Wimplicit-fallthrough
            -Winconsistent-missing-destructor-override
            -Wabstract-vbase-init
            -Warc-repeated-use-of-weak
            -Wassign-enum
            -Watomic-properties
            -Watomic-implicit-seq-cst
            -Wbad-function-cast
            -Wbind-to-temporary-copy
            -Wcomma
            -Wc++-compat
            -Wconditional-uninitialized
            -Wconsumed
            -Wdeprecated-implementations
            -Wdirect-ivar-access
            -Wdisabled-macro-expansion
            # -Wdocumentation # TODO:
            -Wduplicate-decl-specifier
            -Wduplicate-enum
            -Wduplicate-method-arg
            -Wduplicate-method-match
            -Wexpansion-to-defined # enabled by the -Wextra on gcc
            -Wexplicit-ownership-type
            -Wloop-analysis
            -Wextra-semi
            -Wextra-semi-stmt
            -Wheader-hygiene
            -Wimplicit-retain-self
            -Wimport-preprocessor-directive-pedantic
            -Winvalid-or-nonexistent-directory
            -Wmain # enabled by the -Wpedantic on gcc
            -Wmethod-signatures
            -Wmicrosoft
            -Wnarrowing # enabled by the -Wpedantic on gcc
            -Wnewline-eof
            -Wover-aligned
            -Woverriding-method-mismatch
            -Wunguarded-availability
            -Wunused-member-function
            -Wused-but-marked-unused
            -Wvector-conversion
            -Wvla
            -Wunreachable-code-aggressive
            -Wunsupported-dll-base-class-template
            -Wunused-template
            -Wunused-exception-parameter
            -ftemplate-backtrace-limit=0
            -fcolor-diagnostics
            -fansi-escape-codes)
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wframe-address # enabled by the -Wall on gcc
                -Wdtor-name)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wcalled-once-parameter
                -Wcompound-token-split)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wcast-function-type)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Warray-parameter) # enabled as -Warray-parameter=2 by the -Wall on gcc
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wincompatible-function-pointer-types-strict)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 17.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wincompatible-function-pointer-types-strict)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wunsafe-buffer-usage-in-container)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wfunction-effects)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 21.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wshift-bool
                -Wnrvo
                -Wunique-object-duplication)
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 22.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wfunction-effect-redeclarations)
        endif()
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _LIBCPP_ENABLE_ASSERTIONS)
    endif()
    if(NOT USING_MINGW_GCC AND CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -fstack-protector-strong
        )
    endif()

    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
        ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
    )

    # Flags for C only
    set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
        -Wwrite-strings
        -Wint-conversion
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            -Wbad-function-cast # added on the C/C++ stage if Clang is used
            -Wc++-compat # added on the C/C++ stage if Clang is used
            -Wstrict-prototypes # enabled by default if Clang is used
            -Wold-style-definition
            -Wnested-externs
        )
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
            set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
                -fanalyzer
            )
        endif()
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
            -Wc11-extensions
        )
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18.1.0)
            set(LOCAL_FN_TEST_C_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_C_COMPILE_OPTIONS}
                -Wc23-compat
                -Wc23-extensions
            )
        endif()
    endif()
    # Flags for C++ only
    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
        -Weffc++
        -Wsign-promo
        -Wold-style-cast
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Wregister
        )
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wcomma-subscript
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -Wchanges-meaning
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0.0)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -fanalyzer
            )
        endif()
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            -Wsigned-enum-bitfield
        )
    endif()

    set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
        ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
        -U_GLIBCXX_USE_DEPRECATED)
    if (ENABLE_RUNTIME_CHECKS_FLAG__)
        set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
            ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
            _GLIBCXX_SANITIZE_VECTOR
        )
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0)
            # In gcc with version < 10.0 these checks break `constexpr`-tivity of some std:: functions
            set(LOCAL_FN_TEST_COMPILE_DEFINITIONS
                ${LOCAL_FN_TEST_COMPILE_DEFINITIONS}
                _GLIBCXX_DEBUG
                _GLIBCXX_DEBUG_PEDANTIC
                _GLIBCXX_CONCEPT_CHECKS
            )
        endif()

        if(NOT USING_MINGW_GCC)
            set (LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO "${LOCAL_FN_CMAKE_C_FLAGS_RELWITHDEBINFO} -fsanitize=address,undefined" PARENT_SCOPE)
            set (LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO "${LOCAL_FN_CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=address,undefined" PARENT_SCOPE)
        endif()
    endif()

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

    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(ENABLE_RUNTIME_CHECKS_FLAG__)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                -UNDEBUG
            )
        endif()

        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            "/W4" # Enable -Wall and -Wextra
        )
        set(LINK_COMPILER_RT_BUILTINS_MANUALLY True PARENT_SCOPE)
    else()
        if(ENABLE_RUNTIME_CHECKS_FLAG__)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                "/UNDEBUG"
            )
        endif()

        if(MSVC_VERSION GREATER_EQUAL 1914)
            set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
                ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
                "/Zc:__cplusplus")
        endif()
        set(LOCAL_FN_TEST_CXX_COMPILE_OPTIONS
            ${LOCAL_FN_TEST_CXX_COMPILE_OPTIONS}
            "/W4"
            "/wd4146" # Disable C4146: unary minus operator applied to unsigned type, result still unsigned
        )
    endif()

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
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        configure_msvc_or_clang_msvc_options()
    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
        configure_gcc_or_clang_gcc_options()
    else()
        message(FATAL_ERROR "Clang with unknown frontend")
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 4.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 11)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 17)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 20)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 23)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 23)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18.1.2 AND
        NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(COMPILER_SUPPORTS_MODULES True)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 21.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 26)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    configure_gcc_or_clang_gcc_options()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 11)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 17)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 20)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 23)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
        list(APPEND COMPILER_SUPPORTED_C_VERSIONS 23)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 15.0.0)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 26)
        set(COMPILER_SUPPORTS_MODULES True)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    configure_msvc_or_clang_msvc_options()
    if(MSVC_VERSION GREATER_EQUAL 1928)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 20)
    endif()
    if(MSVC_VERSION GREATER_EQUAL 1936)
        set(COMPILER_SUPPORTS_MODULES True)
    endif()
    if(MSVC_VERSION GREATER_EQUAL 1937)
        list(APPEND COMPILER_SUPPORTED_CXX_VERSIONS 23)
    endif()
endif()

function(manually_add_byte_order_to_cppcheck_arguments)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_BYTE_ORDER STREQUAL "LITTLE_ENDIAN")
            list(APPEND CMAKE_CXX_CPPCHECK
                "-D__ORDER_LITTLE_ENDIAN__=1234"
                "-D__ORDER_BIG_ENDIAN__=4321"
                "-D__BYTE_ORDER__=1234"
            )
        elseif(CMAKE_CXX_BYTE_ORDER STREQUAL "BIG_ENDIAN")
            list(APPEND CMAKE_CXX_CPPCHECK
                "-D__ORDER_LITTLE_ENDIAN__=1234"
                "-D__ORDER_BIG_ENDIAN__=4321"
                "-D__BYTE_ORDER__=4321"
            )
        endif()
    endif()
endfunction(manually_add_byte_order_to_cppcheck_arguments)

if(CMAKE_SUPPORTS_MODULES AND GENERATOR_SUPPORTS_MODULES AND COMPILER_SUPPORTS_MODULES)
    set(CXX_MODULES_SUPPORTED True)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(USE_CPPCHECK_DURING_BUILD)
        find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
        if(CMAKE_CXX_CPPCHECK)
            message(STATUS "Found cppcheck: ${CMAKE_CXX_CPPCHECK}")
            set(CPPCHECK_EXITCODE_ON_ERROR 0)
            list(APPEND
                CMAKE_CXX_CPPCHECK
                "--force"
                "--template=gcc"
                "--inline-suppr"
            )
        else()
            message(WARNING "Could not find cppcheck")
        endif()
    endif()

    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(USE_CLANG_TIDY_DURING_BUILD)
            message(STATUS "Compiler is not clang, clang-tidy won't be used")
            set(USE_CLANG_TIDY_DURING_BUILD False)
        endif()
    endif()

    if(USE_CLANG_TIDY_DURING_BUILD)
        find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)

        if(CLANG_TIDY_COMMAND)
            message(STATUS "Found clang-tidy: ${CLANG_TIDY_COMMAND}")
            set(CMAKE_CXX_CLANG_TIDY
                ${CLANG_TIDY_COMMAND}
                "-config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
                --warnings-as-errors=*
                --use-color
                --header-filter=.*)
            if(CXX_MODULES_SUPPORTED)
                set(CMAKE_CXX_CLANG_TIDY
                    ${CMAKE_CXX_CLANG_TIDY}
                    --enable-module-headers-parsing)
            endif()
        else()
            message(WARNING "Could not find clang-tidy")
        endif()
    endif()

    find_program(LSB_RELEASE_EXEC lsb_release)
    if(NOT LSB_RELEASE_EXEC)
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
        if(LSB_DISTRIBUTOR_ID_SHORT STREQUAL "ubuntu" AND LSB_RELEASE_VERSION_SHORT VERSION_LESS_EQUAL "20.04")
            set(LINK_THREADS_LIBRARY_MANUALLY True)
            set(THREADS_PREFER_PTHREAD_FLAG ON)
            find_package(Threads REQUIRED)
        endif()
        if(USE_CPPCHECK_DURING_BUILD AND CMAKE_CXX_CPPCHECK)
            if(LSB_DISTRIBUTOR_ID_SHORT STREQUAL "ubuntu" AND LSB_RELEASE_VERSION_SHORT VERSION_GREATER_EQUAL "24.04")
                if(NOT CXX_MODULES_SUPPORTED)
                    set(CPPCHECK_EXITCODE_ON_ERROR 1)
                endif()
                list(APPEND
                    CMAKE_CXX_CPPCHECK
                    "--check-level=exhaustive"
                )
            endif()
        endif()
    endif()

    if(USE_CPPCHECK_DURING_BUILD AND CMAKE_CXX_CPPCHECK)
        list(APPEND
            CMAKE_CXX_CPPCHECK
            "--error-exitcode=${CPPCHECK_EXITCODE_ON_ERROR}"
        )
    endif()
endif()

message(STATUS "+-")
message(STATUS "| TEST_COMPILE_DEFINITIONS = ${TEST_COMPILE_DEFINITIONS}")
message(STATUS "| TEST_C_COMPILE_OPTIONS = ${TEST_C_COMPILE_OPTIONS}")
message(STATUS "| TEST_CXX_COMPILE_OPTIONS = ${TEST_CXX_COMPILE_OPTIONS}")
message(STATUS "| USING_MINGW_GCC = ${USING_MINGW_GCC}")
message(STATUS "| USING_MINGW_GCC_32 = ${USING_MINGW_GCC_32}")
message(STATUS "| USING_MSYS2_MINGW_GCC = ${USING_MSYS2_MINGW_GCC}")
message(STATUS "| COMPILER_SUPPORTED_C_VERSIONS = ${COMPILER_SUPPORTED_C_VERSIONS}")
message(STATUS "| COMPILER_SUPPORTED_CXX_VERSIONS = ${COMPILER_SUPPORTED_CXX_VERSIONS}")
message(STATUS "| CXX_MODULES_SUPPORTED = ${CXX_MODULES_SUPPORTED}")
message(STATUS "| CMAKE_SUPPORTS_MODULES = ${CMAKE_SUPPORTS_MODULES}")
message(STATUS "| GENERATOR_SUPPORTS_MODULES = ${GENERATOR_SUPPORTS_MODULES}")
message(STATUS "| COMPILER_SUPPORTS_MODULES = ${COMPILER_SUPPORTS_MODULES}")
message(STATUS "| CMAKE_CXX_CLANG_TIDY = ${CMAKE_CXX_CLANG_TIDY}")
message(STATUS "| CMAKE_CXX_CPPCHECK = ${CMAKE_CXX_CPPCHECK}")
message(STATUS "| ENABLE_RUNTIME_CHECKS_FLAG = ${ENABLE_RUNTIME_CHECKS_FLAG__}")
message(STATUS "+-")
