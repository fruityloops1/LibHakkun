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

list(FIND HAKKUN_ADDONS Nvn HAS_NVN)
if (HAS_NVN EQUAL -1)
    message(FATAL_ERROR "Enable Nvn addon to use ImGui")
endif()

if (NVN_TARGET STREQUAL Nvn2)
    embed_file(ImGui ${CMAKE_CURRENT_SOURCE_DIR}/data/ImGui.Nvn2.bin ImGuiShader 0x1000 TRUE)
else()
    embed_file(ImGui ${CMAKE_CURRENT_SOURCE_DIR}/data/ImGui.Nvn.bin ImGuiShader 0x1000 TRUE)
endif()

apply_config(ImGui)

target_compile_definitions(ImGui PRIVATE IMGUI_DISABLE_TTY_FUNCTIONS IMGUI_DISABLE_TIME_FUNCTIONS IMGUI_DISABLE_DEFAULT_ALLOCATORS IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/include/hk/gfx/ImGuiConfig.h")
target_include_directories(ImGui PRIVATE include/hk ${CMAKE_CURRENT_BINARY_DIR} ${IMGUI_DIR})
