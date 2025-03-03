cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/nvn/nvn_CppFuncPtrImpl.cpp

    src/hk/gfx/ShaderImpl.cpp
    src/hk/gfx/TextureImpl.cpp
    )
add_library(Nvn ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(Nvn)

target_compile_definitions(Nvn PRIVATE)
target_include_directories(Nvn PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
