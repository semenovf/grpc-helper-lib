################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `grpc-helper-lib`.
#
# Changelog:
#      2022.06.15 Initial version.
################################################################################
project(grpc-helper-lib-TESTS CXX C)

portable_target(ADD_TEST async SOURCES async/async.cpp LINK pfs::grpc_helper)
portable_target(LINK_GRPC async
    PROTOC_BIN ${GRPC_HELPER__PROTOC_BIN}
    CPP_PLUGIN ${GRPC_HELPER__CPP_PLUGIN}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto
    SOURCES async/async.proto)

portable_target(ADD_TEST connection_recovery
    SOURCES connection_recovery/connection_recovery.cpp LINK pfs::grpc_helper)
portable_target(LINK_GRPC connection_recovery
    PROTOC_BIN ${GRPC_HELPER__PROTOC_BIN}
    CPP_PLUGIN ${GRPC_HELPER__CPP_PLUGIN}
    OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto
    SOURCES connection_recovery/connection_recovery.proto)
