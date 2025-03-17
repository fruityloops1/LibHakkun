cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/mem/BssHeap.cpp
    src/hk/mem/BssHeapWrappers.cpp
    )
add_library(HeapSourceBss ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

list(FIND HAKKUN_ADDONS HeapSourceDynamic HAS_DYNHEAP)
if (NOT HAS_DYNHEAP EQUAL -1)
    message(FATAL_ERROR "HeapSourceBss and HeapSourceDynamic cannot be used at the same time")
endif()

list(FIND HAKKUN_ADDONS ExpHeap HAS_EXPHEAP)
if (HAS_EXPHEAP EQUAL -1)
    message(FATAL_ERROR "Enable ExpHeap addon to use HeapSourceBss")
endif()

if (HAKKUN_MAIN_HEAP_USER_ARENA)
    target_compile_definitions(HeapSourceBss PRIVATE HAKKUN_MAIN_HEAP_USER_ARENA)
    message("Using user arena for main heap")
else()
    if (NOT DEFINED HAKKUN_BSS_HEAP_SIZE)
        message(WARNING "HAKKUN_BSS_HEAP_SIZE not defined, using 256KB")
        set(HAKKUN_BSS_HEAP_SIZE 0x40000)
    endif()
endif()

apply_config(HeapSourceBss)

target_compile_definitions(HeapSourceBss PRIVATE HAKKUN_BSS_HEAP_SIZE=${HAKKUN_BSS_HEAP_SIZE})
target_include_directories(HeapSourceBss PRIVATE include src ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
