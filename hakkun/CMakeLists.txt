cmake_minimum_required(VERSION 3.16)

include_directories(include)
set(SOURCES
    src/hk/diag/diag.cpp

    src/hk/hook/MapUtil.cpp
    src/hk/hook/Trampoline.cpp

    src/hk/init/module.S
    src/hk/init/module.cpp

    src/hk/ro/RoUtil.cpp
    src/hk/ro/RoModule.cpp

    src/hk/sail/detail.cpp
    src/hk/sail/init.cpp

    src/hk/util/Context.cpp

    src/hk/svc/api.S

    src/rtld/RoModule.cpp
    )
add_library(LibHakkun ${SOURCES})

include(../../config/config.cmake)
include(../cmake/apply_config.cmake)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_options(LibHakkun PRIVATE ${OPTIMIZE_OPTIONS_RELEASE})
else()
    target_compile_options(LibHakkun PRIVATE ${OPTIMIZE_OPTIONS_DEBUG})
endif()

apply_config(LibHakkun)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
