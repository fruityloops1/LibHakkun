cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/gfx/DebugRenderer.cpp
    src/hk/gfx/Font.cpp
    )
add_library(DebugRenderer ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)
include(../../cmake/bin2s.cmake)

list(FIND HAKKUN_ADDONS Nvn HAS_NVN)
if (HAS_NVN EQUAL -1)
    message(FATAL_ERROR "Enable Nvn addon to use DebugRenderer")
endif()

embed_file(DebugRenderer ${CMAKE_CURRENT_SOURCE_DIR}/data/shader.bin shader 0x1000 TRUE)
embed_file(DebugRenderer ${CMAKE_CURRENT_SOURCE_DIR}/data/font.hkf font 0x1000 TRUE)

apply_config(DebugRenderer)

target_compile_definitions(DebugRenderer PRIVATE)
target_include_directories(DebugRenderer PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR})

if (NOT DEFINED HAKKUN_DEBUGRENDERER_VTXBUFFER_SIZE)
    message(WARNING "HAKKUN_DEBUGRENDERER_VTXBUFFER_SIZE not defined, using 0x1000")
    set(HAKKUN_DEBUGRENDERER_VTXBUFFER_SIZE 0x1000)
endif()

target_compile_definitions(DebugRenderer PUBLIC HAKKUN_DEBUGRENDERER_VTXBUFFER_SIZE=${HAKKUN_DEBUGRENDERER_VTXBUFFER_SIZE})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
