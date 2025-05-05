cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    include/hk/nn/diag.h
    )
add_library(LogManager ${SOURCES})
set_target_properties(LogManager PROPERTIES LINKER_LANGUAGE CXX)

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(LogManager)

target_compile_definitions(LogManager PRIVATE)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
