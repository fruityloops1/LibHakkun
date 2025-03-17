cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
        src/hk/test.cpp
    )
add_library(ExpHeap ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(ExpHeap)

target_compile_definitions(ExpHeap PRIVATE)
target_include_directories(ExpHeap PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
