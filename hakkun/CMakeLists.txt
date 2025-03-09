cmake_minimum_required(VERSION 3.16)

include(../../config/config.cmake)
include(../cmake/apply_config.cmake)

include_directories(include)
set(SOURCES
    src/hk/diag/diag.cpp

    src/hk/hook/MapUtil.cpp
    src/hk/hook/Trampoline.cpp

    src/hk/init/module.S
    src/hk/init/module.cpp

    src/hk/os/Libcxx.cpp

    src/hk/ro/RoUtil.cpp
    src/hk/ro/RoModule.cpp

    src/hk/sail/detail.cpp
    src/hk/sail/init.cpp

    src/hk/util/Context.cpp

    src/rtld/RoModule.cpp
    )

if (IS_32_BIT)
    set(SOURCES ${SOURCES}
        src/hk/svc/api.armv7a.S
        )
else()
    set(SOURCES ${SOURCES}
        src/hk/svc/api.aarch64.S
        )
endif()

add_library(LibHakkun ${SOURCES})

apply_config(LibHakkun)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
