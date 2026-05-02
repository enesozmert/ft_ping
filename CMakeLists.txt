# =============================================================================
# ft_ping — CMakeLists.txt (paralel build sistemi, IDE entegrasyonu için)
# Subject: Ecole 42 ft_ping v5.1
# Resmi build sistemi Makefile'dır; CMake yardımcı/IDE içindir.
# =============================================================================
cmake_minimum_required(VERSION 3.16)

project(ft_ping
    VERSION     5.1
    DESCRIPTION "Ecole 42 — ICMP ping recoder"
    LANGUAGES   C
)

# -----------------------------------------------------------------------------
# C standard & build type
# -----------------------------------------------------------------------------
set(CMAKE_C_STANDARD          11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# -----------------------------------------------------------------------------
# Subject mandatory warning flags
# -----------------------------------------------------------------------------
add_compile_options(
    -Wall
    -Wextra
    -Werror
)

# Strict flag set (FT_PING_STRICT=ON)
option(FT_PING_STRICT "Enable extra strict warnings" OFF)
if(FT_PING_STRICT)
    add_compile_options(
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wcast-align
        -Wstrict-prototypes
        -Wmissing-prototypes
    )
endif()

# AddressSanitizer (FT_PING_ASAN=ON)
option(FT_PING_ASAN "Build with AddressSanitizer" OFF)
if(FT_PING_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# Debug build flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
endif()

# -----------------------------------------------------------------------------
# Source discovery
# -----------------------------------------------------------------------------
file(GLOB FT_PING_SRCS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB FT_PING_HDRS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/hdr/*.h")

if(NOT FT_PING_SRCS)
    message(FATAL_ERROR "No source files found in src/")
endif()

# -----------------------------------------------------------------------------
# Executable
# -----------------------------------------------------------------------------
add_executable(ft_ping ${FT_PING_SRCS})

target_include_directories(ft_ping PRIVATE
    ${CMAKE_SOURCE_DIR}/hdr
)

# Place binary into <project>/output (Makefile parity)
set_target_properties(ft_ping PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/output"
)

# Linux/POSIX feature macros
if(UNIX AND NOT APPLE)
    target_compile_definitions(ft_ping PRIVATE _GNU_SOURCE)
endif()

# Math library (sqrt for mdev statistics)
target_link_libraries(ft_ping PRIVATE m)

# -----------------------------------------------------------------------------
# Custom target — Norminette (Ecole 42)
# -----------------------------------------------------------------------------
find_program(NORMINETTE_EXE norminette)
if(NORMINETTE_EXE)
    add_custom_target(norm
        COMMAND ${NORMINETTE_EXE} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/hdr
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Ecole 42 norminette..."
        VERBATIM
    )
else()
    add_custom_target(norm
        COMMAND ${CMAKE_COMMAND} -E echo
            "norminette not installed — run: pip install norminette"
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "norminette not available"
    )
endif()

# -----------------------------------------------------------------------------
# Custom target — Cppcheck (general)
# -----------------------------------------------------------------------------
find_program(CPPCHECK_EXE cppcheck)
if(CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --inconclusive
            --std=c11
            --force
            --suppress=missingIncludeSystem
            -I ${CMAKE_SOURCE_DIR}/hdr
            --quiet
            ${CMAKE_SOURCE_DIR}/src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running cppcheck..."
        VERBATIM
    )

    # MISRA C:2012 addon
    find_program(PYTHON3_EXE python3)
    set(MISRA_ADDON "/usr/share/cppcheck/addons/misra.py"
        CACHE FILEPATH "Path to misra.py addon")

    add_custom_target(misra
        COMMAND ${CPPCHECK_EXE}
            --addon=misra
            --std=c11
            -I ${CMAKE_SOURCE_DIR}/hdr
            ${CMAKE_SOURCE_DIR}/src
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running MISRA C:2012 (cppcheck addon)..."
        VERBATIM
    )
else()
    add_custom_target(cppcheck
        COMMAND ${CMAKE_COMMAND} -E echo
            "cppcheck not installed — run: apt-get install cppcheck"
        COMMENT "cppcheck not available"
    )
    add_custom_target(misra
        COMMAND ${CMAKE_COMMAND} -E echo
            "cppcheck (MISRA addon) not installed"
        COMMENT "MISRA addon not available"
    )
endif()

# -----------------------------------------------------------------------------
# Custom target — Run (with sudo)
# -----------------------------------------------------------------------------
set(FT_PING_ARGS "" CACHE STRING "Arguments passed to ft_ping run target")
add_custom_target(run
    COMMAND sudo $<TARGET_FILE:ft_ping> ${FT_PING_ARGS}
    DEPENDS ft_ping
    USES_TERMINAL
    COMMENT "Running ft_ping (sudo)"
)

# Apply CAP_NET_RAW capability to the binary
add_custom_target(setcap
    COMMAND sudo setcap cap_net_raw+ep $<TARGET_FILE:ft_ping>
    DEPENDS ft_ping
    COMMENT "Applying CAP_NET_RAW to ft_ping binary"
)

# -----------------------------------------------------------------------------
# Custom target — Valgrind
# -----------------------------------------------------------------------------
find_program(VALGRIND_EXE valgrind)
set(FT_PING_VALGRIND_HOST "127.0.0.1"
    CACHE STRING "Host used by valgrind target")

if(VALGRIND_EXE)
    add_custom_target(valgrind
        COMMAND sudo ${VALGRIND_EXE}
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --error-exitcode=1
            $<TARGET_FILE:ft_ping> -c 2 ${FT_PING_VALGRIND_HOST}
        DEPENDS ft_ping
        USES_TERMINAL
        COMMENT "Running valgrind (sudo)"
    )
else()
    add_custom_target(valgrind
        COMMAND ${CMAKE_COMMAND} -E echo
            "valgrind not installed — run: apt-get install valgrind"
        COMMENT "valgrind not available"
    )
endif()

# -----------------------------------------------------------------------------
# Tests — CTest entegrasyonu
# -----------------------------------------------------------------------------
enable_testing()

# T-505: -? usage flag returns success
add_test(NAME help_flag
    COMMAND ft_ping -?
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# T-511: no arguments → non-zero exit
add_test(NAME no_args
    COMMAND ft_ping
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(no_args PROPERTIES WILL_FAIL TRUE)

# T-510: unknown host
add_test(NAME unknown_host
    COMMAND ft_ping nonexistent.invalid.tld
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(unknown_host PROPERTIES WILL_FAIL TRUE)

# T-512: invalid flag
add_test(NAME invalid_flag
    COMMAND ft_ping -X 8.8.8.8
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
set_tests_properties(invalid_flag PROPERTIES WILL_FAIL TRUE)

# Custom shell tests under test/ (if directory exists)
if(EXISTS "${CMAKE_SOURCE_DIR}/test")
    file(GLOB TEST_SCRIPTS "${CMAKE_SOURCE_DIR}/test/*.sh")
    foreach(script ${TEST_SCRIPTS})
        get_filename_component(test_name ${script} NAME_WE)
        add_test(NAME script_${test_name}
            COMMAND sh ${script}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )
    endforeach()
endif()

# Convenience target: build + ctest --output-on-failure
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ft_ping
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
    COMMENT "Running CTest suite"
)

# -----------------------------------------------------------------------------
# Quality umbrella: norm + cppcheck + check
# -----------------------------------------------------------------------------
add_custom_target(quality
    DEPENDS norm cppcheck check
    COMMENT "Running norm + cppcheck + tests"
)

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
message(STATUS "ft_ping configuration:")
message(STATUS "  Source files: ${FT_PING_SRCS}")
message(STATUS "  Output dir  : ${CMAKE_SOURCE_DIR}/output")
message(STATUS "  norminette  : ${NORMINETTE_EXE}")
message(STATUS "  cppcheck    : ${CPPCHECK_EXE}")
message(STATUS "  valgrind    : ${VALGRIND_EXE}")
message(STATUS "  Strict      : ${FT_PING_STRICT}")
message(STATUS "  ASan        : ${FT_PING_ASAN}")
