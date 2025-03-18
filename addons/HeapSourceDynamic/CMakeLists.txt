cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/mem/DynamicHeapWrappers.cpp
    )
add_library(HeapSourceDynamic ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

list(FIND HAKKUN_ADDONS HeapSourceBss HAS_BSSHEAP)
if (NOT HAS_BSSHEAP EQUAL -1)
    message(FATAL_ERROR "HeapSourceDynamic and HeapSourceBss cannot be used at the same time")
endif()

apply_config(HeapSourceDynamic)

target_compile_definitions(HeapSourceDynamic PRIVATE)
target_include_directories(HeapSourceDynamic PRIVATE include src ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
