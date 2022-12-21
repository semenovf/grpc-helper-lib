################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `grpc-helper-lib`.
#
# Changelog:
#      2022.06.15 Initial version (inspired from `pfs-grpc`).
#      2022.07.17 Moved gRPC build instructions into separate file:
#                 cmake/gRPC.cmake.
################################################################################
################################################################################
# Copyright (c) 2019 Vladislav Trifochkin
#
# This file is part of `pfs-grpc`.
#
################################################################################
################################################################################
#
# This sctipt choose appropriate version of gRPC: preloaded (with modifications
# to use with GCC 4.7.2) or loaded as submodule
#
################################################################################
cmake_minimum_required (VERSION 3.11)
project(grpc-helper C CXX)

option(GRPC_HELPER__FORCE_PRELOADED_GRPC "Force process preloaded version of gRPC" OFF)

# `grpc-helper` source directory
set(GRPC_HELPER__ROOT "${CMAKE_CURRENT_LIST_DIR}")

portable_target(ADD_INTERFACE ${PROJECT_NAME} ALIAS pfs::grpc-helper)
portable_target(INCLUDE_DIRS ${PROJECT_NAME} INTERFACE ${GRPC_HELPER__ROOT}/include)

portable_target(INCLUDE_PROJECT
    ${PORTABLE_TARGET__CURRENT_PROJECT_DIR}/cmake/gRPC.cmake)
