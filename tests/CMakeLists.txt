################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `grpc-helper-lib`.
#
# Changelog:
#      2022.06.15 Initial version.
################################################################################
project(grpc-helper-TESTS CXX C)

portable_target(ADD_TEST async SOURCES async/async.cpp LINK pfs::grpc_helper)
portable_target(LINK_GRPC async
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto
    PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/async
    SOURCES async.proto)

portable_target(ADD_TEST connection_recovery
    SOURCES connection_recovery/connection_recovery.cpp LINK pfs::grpc_helper)
portable_target(LINK_GRPC connection_recovery
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto
    PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/connection_recovery
    SOURCES connection_recovery.proto)
