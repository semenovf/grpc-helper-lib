################################################################################
# Copyright (c) 2019-2022 Vladislav Trifochkin
#
# This file is part of `grpc-helper-lib`.
#
# Changelog:
#      2022.07.17 Initial version.
################################################################################
cmake_minimum_required(VERSION 3.11)

set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "Use CMake C++ standard meta features (e.g. cxx_std_11) that propagate to targets that link to Abseil")
set(gRPC_BUILD_TESTS OFF CACHE BOOL "Build tests")
set(CARES_BUILD_TOOLS OFF CACHE BOOL "C-ares build tools")

# Workaround for GCC 4.7.2
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    include(CheckCXXSourceCompiles)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx_gxx_permissive.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_is_trivially_destructible.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_map_emplace.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_unordered_map_emplace.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/cxx11_gcc_47_compiler_error_1035.cmake)
endif()

# Submodules downloaded (allow use of last version of gRPC library and
# it's dependences)
if (NOT GRPC_HELPER__FORCE_PRELOADED_GRPC AND EXISTS "${GRPC_HELPER__ROOT}/3rdparty/grpc/CMakeLists.txt")
    # OUTPUT VARIABLE: root gRPC source directory
    set(GRPC_HELPER__GRPC_SOURCE_DIR "${GRPC_HELPER__ROOT}/3rdparty/grpc")
    set(GRPC_HELPER__GRPC_BINARY_SUBDIR "3rdparty/grpc")
else()
    # OUTPUT VARIABLE: root gRPC source directory
    set(GRPC_HELPER__GRPC_SOURCE_DIR "${GRPC_HELPER__ROOT}/3rdparty/preloaded/grpc")
    set(GRPC_HELPER__GRPC_BINARY_SUBDIR "3rdparty/preloaded/grpc")
endif()

if (ANDROID)
    portable_target(GET PROTOC_BIN _gRPC_PROTOBUF_PROTOC_EXECUTABLE)
    portable_target(GET GRPC_CPP_PLUGIN _gRPC_CPP_PLUGIN)

    if (NOT _gRPC_PROTOBUF_PROTOC_EXECUTABLE)
        message(FATAL_ERROR "`protoc` path must be specified")
    endif()

    if (NOT _gRPC_CPP_PLUGIN)
        message(FATAL_ERROR "`grpc_cpp_plugin` path must be specified")
    endif()

    if (NOT EXISTS ${_gRPC_PROTOBUF_PROTOC_EXECUTABLE})
        message(FATAL_ERROR "Invalid location for `protoc`: ${_gRPC_PROTOBUF_PROTOC_EXECUTABLE}")
    endif()

    if (NOT EXISTS ${_gRPC_CPP_PLUGIN})
        message(FATAL_ERROR "Invalid location for `grpc_cpp_plugin`: ${_gRPC_CPP_PLUGIN}")
    endif()
endif()

if (MSVC)
    # Build with multiple processes
    #add_definitions(/MP) # --eugene--
    # MSVC warning suppressions
    add_definitions(
        /wd4018 # 'expression' : signed/unsigned mismatch
        /wd4800 # 'type' : forcing value to bool 'true' or 'false' (performance warning)
        /wd4146 # unary minus operator applied to unsigned type, result still unsigned
        /wd4334 # 'operator' : result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
        /wd4065 # switch statement contains 'default' but no 'case' labels
        /wd4244 # 'conversion' conversion from 'type1' to 'type2', possible loss of data
        /wd4251 # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
        /wd4267 # 'var' : conversion from 'size_t' to 'type', possible loss of data
        /wd4305 # 'identifier' : truncation from 'type1' to 'type2'
        /wd4307 # 'operator' : integral constant overflow
        /wd4309 # 'conversion' : truncation of constant value
        /wd4355 # 'this' : used in base member initializer list
        /wd4506 # no definition for inline function 'function'
        /wd4996 # The compiler encountered a deprecated declaration.
        /wd5208 # unnamed class used in typedef name cannot declare members other than non-static data members, member enumerations, or member classes
    )
endif(MSVC)

# Build gRPC library with `protoc` compiler and gRPC C++ plugin `grpc_cpp_plugin`
# Builds static library for now.
add_subdirectory(${GRPC_HELPER__GRPC_SOURCE_DIR} ${GRPC_HELPER__GRPC_BINARY_SUBDIR})

# Workaround for GCC 4.7.2
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    target_compile_definitions(gpr PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc_cronet PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc_unsecure PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc++ PRIVATE "-D__STDC_LIMIT_MACROS")
    target_compile_definitions(grpc++_unsecure PRIVATE "-D__STDC_LIMIT_MACROS")
endif()

# Workaround for ANDROID (boringssl)
if (ANDROID)
    target_compile_options(crypto_base PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(cipher_extra PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(bio PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(asn1 PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(cast PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(fipsmodule PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(blowfish PRIVATE "-Wno-implicit-fallthrough")
    target_compile_options(des_decrepit PRIVATE "-Wno-implicit-fallthrough")
endif()

# if (NOT ANDROID)
#     if (NOT CUSTOM_PROTOC)
#         add_dependencies(${PROJECT_NAME} protoc grpc_cpp_plugin)
#     endif()
#
#     if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
#         set(GRPC_HELPER__PROTOC_BIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
#         set(GRPC_HELPER__CPP_PLUGIN "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
#     else()
#         set_target_properties(protoc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools")
#         set_target_properties(grpc_cpp_plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools")
#
#         set(GRPC_HELPER__PROTOC_BIN "${CMAKE_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/protoc${CMAKE_EXECUTABLE_SUFFIX}")
#         set(GRPC_HELPER__CPP_PLUGIN "${CMAKE_BINARY_DIR}/tools/${CMAKE_CFG_INTDIR}/grpc_cpp_plugin${CMAKE_EXECUTABLE_SUFFIX}")
#     endif()
# endif()
#
# message(STATUS "Protobuf compiler      : ${GRPC_HELPER__PROTOC_BIN}")
# message(STATUS "gRPC source dir        : ${GRPC_HELPER__GRPC_SOURCE_DIR}")
# message(STATUS "gRPC C++ plugin        : ${GRPC_HELPER__CPP_PLUGIN}")
