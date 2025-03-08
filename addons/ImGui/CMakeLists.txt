cmake_minimum_required(VERSION 3.16)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
set(IMGUI_DIR ${ROOTDIR}/lib/imgui)
if (NOT IS_DIRECTORY ${IMGUI_DIR})
    message(FATAL_ERROR "Please clone imgui into ${IMGUI_DIR}")
endif()

include_directories(include)
set(SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp

    src/hk/gfx/ImGuiBackendNvn.cpp
    )
add_library(ImGui ${SOURCES})

include(../../../config/config.cmake)
include(../../cmake/apply_config.cmake)
include(../../cmake/bin2s.cmake)

embed_file(ImGui ${CMAKE_CURRENT_SOURCE_DIR}/data/shader.bin shader)

apply_config(ImGui)

target_compile_definitions(ImGui PRIVATE IMGUI_DISABLE_DEFAULT_ALLOCATORS IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/include/hk/gfx/ImGuiConfig.h")
target_include_directories(ImGui PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR} ${IMGUI_DIR})
