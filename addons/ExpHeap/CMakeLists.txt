cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/ams/lmem/lmem_common.cpp
    src/ams/lmem/lmem_exp_heap.cpp

    src/ams/lmem/impl/lmem_impl_common_heap.cpp
    src/ams/lmem/impl/lmem_impl_exp_heap.cpp

    src/hk/mem/ExpHeap.cpp
    )
add_library(ExpHeap ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(ExpHeap)

target_compile_definitions(ExpHeap PRIVATE)
target_include_directories(ExpHeap PRIVATE include src ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
