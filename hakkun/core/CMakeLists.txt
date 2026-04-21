cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED HAKKUN_CORE_CONFIG_FILE)
    message(FATAL_ERROR "Set HAKKUN_CORE_CONFIG_FILE")
endif()

include(${HAKKUN_CORE_CONFIG_FILE})
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/apply_config_core.cmake)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    src/hk/Result.cpp
    src/hk/diag/diag.cpp
    src/hk/diag/ResultName.cpp
    )

add_library(LibHakkunCore ${SOURCES})
target_include_directories(LibHakkunCore PUBLIC include)

apply_config_core(LibHakkunCore)
