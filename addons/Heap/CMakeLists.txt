cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/heap.cpp
)
add_library(Heap ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(Heap)

target_compile_definitions(Heap PRIVATE)
target_include_directories(Heap PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
