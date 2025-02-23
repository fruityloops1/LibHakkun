cmake_minimum_required(VERSION 3.16)

file(TO_CMAKE_PATH "$ENV{BIN2S}" BIN2S)

if(NOT EXISTS ${BIN2S})
    if(NOT IS_DIRECTORY ${DEVKITPRO})
        message(FATAL_ERROR "Please install devkitA64 or set BIN2S in your environment.")
    else()
        set(BIN2S ${DEVKITPRO}/tools/bin/bin2s)
    endif()
endif()

include_directories(include)
set(SOURCES
    src/hk/nvn/nvn_CppFuncPtrImpl.cpp
    src/hk/gfx/DebugRenderer.cpp
    )
add_library(DebugRenderer ${SOURCES})

function(add_shader shader)
    set(BINPATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader}.bin)
    set(SPATH ${CMAKE_CURRENT_BINARY_DIR}/${shader}_shader.S)

    add_custom_command(OUTPUT ${SPATH} PRE_BUILD
        COMMAND ${BIN2S} ${BINPATH} --header ${CMAKE_CURRENT_BINARY_DIR}/${shader}_shader.h > ${SPATH} && sed -i -f ${CMAKE_CURRENT_SOURCE_DIR}/replace_bin2s.sed ${SPATH}
        )
    set_source_files_properties(${SPATH} PROPERTIES GENERATED TRUE)
    target_sources(DebugRenderer PRIVATE ${SPATH})
endfunction()

add_shader(base)
add_shader(OBAMA)

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)

apply_config(DebugRenderer)

target_compile_definitions(DebugRenderer PRIVATE)
target_include_directories(DebugRenderer PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR})

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
