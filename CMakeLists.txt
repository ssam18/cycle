# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.30...4.3)

include(infra/cmake/enable-experimental-import-std.cmake)

project(
    beman.cycle # CMake Project Name, which is also the name of the top-level
    # targets (e.g., library, executable, etc.).
    DESCRIPTION
        "Range adaptor that endlessly repeats a forward range (views::cycle, P3806R0)."
    LANGUAGES CXX
    VERSION 0.1.0
)

# [CMAKE.SKIP_TESTS]
option(
    BEMAN_CYCLE_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

# [CMAKE.SKIP_EXAMPLES]
option(
    BEMAN_CYCLE_BUILD_EXAMPLES
    "Enable building examples. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(BEMAN_CYCLE_USE_MODULES "Provide beman.cycle as a C++ module" OFF)

if(BEMAN_CYCLE_USE_MODULES)
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/include/beman/cycle/config_generated.hpp.in"
    "${PROJECT_BINARY_DIR}/include/beman/cycle/config_generated.hpp"
    @ONLY
)

# for find of beman_install_library and configure_build_telemetry
include(infra/cmake/beman-install-library.cmake)
include(infra/cmake/BuildTelemetryConfig.cmake)

if(BEMAN_CYCLE_USE_MODULES)
    add_library(beman.cycle STATIC)
else()
    add_library(beman.cycle INTERFACE)
endif()
add_library(beman::cycle ALIAS beman.cycle)

if(BEMAN_CYCLE_USE_MODULES)
    target_compile_features(beman.cycle PUBLIC cxx_std_23)
    set_target_properties(beman.cycle PROPERTIES CXX_MODULE_STD ON)
    target_sources(
        beman.cycle
        PUBLIC
            FILE_SET CXX_MODULES
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
else()
    target_compile_features(beman.cycle INTERFACE cxx_std_23)
    target_sources(
        beman.cycle
        PUBLIC
            FILE_SET HEADERS
                BASE_DIRS
                    "${CMAKE_CURRENT_SOURCE_DIR}/include"
                    "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
endif()

set_target_properties(
    beman.cycle
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS ${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(include/beman/cycle)

beman_install_library(beman.cycle TARGETS beman.cycle)

if(BEMAN_CYCLE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/beman/cycle)
endif()

if(BEMAN_CYCLE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
