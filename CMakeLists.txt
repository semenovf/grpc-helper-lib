################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `grpc-helper-lib`.
#
# Changelog:
#      2022.06.15 Initial version (inspired from `pfs-grpc`).
################################################################################
cmake_minimum_required (VERSION 3.11)
project(grpc-helper-lib CXX C)

option(GRPS_HELPER__BUILD_STRICT "Build with strict policies: C++ standard required, C++ extension is OFF etc" ON)
option(GRPS_HELPER__BUILD_TESTS "Build tests" OFF)
option(GRPS_HELPER__BUILD_DEMO "Build demo" OFF)
option(GRPS_HELPER__ENABLE_COVERAGE "Build tests with coverage support" OFF)

if (NOT PORTABLE_TARGET__ENABLED)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/3rdparty/portable-target/cmake/v2)
    include(PortableTarget)
    portable_target(SET ENABLE_TRACE ON)
endif()

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if (GRPS_HELPER__BUILD_STRICT)
    set(CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF) # use -std=c++XX rather than -std=gnu++XX
endif()

include(${CMAKE_CURRENT_LIST_DIR}/library.cmake)

if (GRPS_HELPER__BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
