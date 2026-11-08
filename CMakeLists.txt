cmake_minimum_required(VERSION 3.22)

project(nxf
    VERSION 0.1.0
    DESCRIPTION "NXF - NUX Framework: an original, JUCE-independent C++ framework for audio applications"
    LANGUAGES CXX)

# NXF is C++20. No exceptions: modules may not quietly require a newer
# standard, because the SDK promise (Phase 12) is a C++20 consumer build.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NXF_BUILD_TESTS "Build NXF tests" ON)
option(NXF_BUILD_EXAMPLES "Build NXF examples" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(NxfWarnings)

# --- Targets -----------------------------------------------------------------
# Phase 0 implements nxf_core only. Other modules exist as documented
# boundaries (docs/ARCHITECTURE.md), not as empty libraries pretending
# to be finished work.

add_library(nxf_core src/core/Version.cpp)
add_library(nxf::core ALIAS nxf_core)
target_include_directories(nxf_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(nxf_core PUBLIC cxx_std_20)
nxf_set_warnings(nxf_core)

if(NXF_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_nxf)
endif()

if(NXF_BUILD_TESTS)
    enable_testing()
    add_executable(nxf_unit_tests tests/unit/CoreVersionTests.cpp)
    target_link_libraries(nxf_unit_tests PRIVATE nxf::core)
    nxf_set_warnings(nxf_unit_tests)
    add_test(NAME nxf.unit.core_version COMMAND nxf_unit_tests)
endif()

message(STATUS "NXF ${PROJECT_VERSION} configured (C++20). Implemented modules: nxf_core. All other modules are planned - see docs/ROADMAP.md.")
