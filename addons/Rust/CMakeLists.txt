cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/interop.cpp
    )
add_library(Rust ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(Rust)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

if(IS_32BIT)
  message(FATAL_ERROR "32-bit modules are not yet supported")
endif()

if(NOT RUST_MANIFEST_PATH)
  message(FATAL_ERROR "You must specify the path to your Cargo.toml in RUST_MANIFEST_PATH")
endif()

if(NOT RUST_LIB_NAME)
  message(FATAL_ERROR "You must specify the name of your library crate")
endif()

if(NOT RUST_TARGET)
  set(RUST_TARGET aarch64-nintendo-switch-freestanding)
endif()

if(NOT RUST_TARGET_DIR)
  set(RUST_TARGET_DIR ${ROOTDIR}/target)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(RUST_PROFILE dev)
  set(RUST_TARGET_PROFILE debug)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(RUST_PROFILE release)
  set(RUST_TARGET_PROFILE release)
else()
  message(FATAL_ERROR "Unsupported build type ${CMAKE_BUILD_TYPE}")
endif()

set(STATIC_LIB_LOCATION "${RUST_TARGET_DIR}/${RUST_TARGET}/${RUST_TARGET_PROFILE}/lib${RUST_LIB_NAME}.a")

add_custom_target(Cargo COMMAND cargo build --manifest-path ${RUST_MANIFEST_PATH} --profile ${RUST_PROFILE})

add_library(RustLib STATIC IMPORTED)
set_target_properties(RustLib PROPERTIES IMPORTED_LOCATION ${STATIC_LIB_LOCATION})
add_dependencies(RustLib Cargo)

target_link_libraries(Rust LINK_PUBLIC RustLib)

target_compile_definitions(Rust PRIVATE)
target_include_directories(Rust PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR})
