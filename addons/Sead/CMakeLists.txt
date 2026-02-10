cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/dummy.cpp
    )
add_library(Sead ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(Sead)

target_compile_definitions(Sead PRIVATE)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
