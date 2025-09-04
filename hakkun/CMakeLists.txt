cmake_minimum_required(VERSION 3.16)

include(../../config/config.cmake)
include(../cmake/apply_config.cmake)

include_directories(include)
set(SOURCES
    src/hk/hook/MapUtil.cpp
    src/hk/hook/Trampoline.cpp

    src/hk/init/mod0.S
    src/hk/init/moduleEntry.S

    src/hk/os/Libcxx.cpp
    src/hk/os/Thread.cpp

    src/hk/ro/RoUtil.cpp
    src/hk/ro/RoModule.cpp

    src/hk/sail/detail.cpp
    src/hk/sail/init.cpp

    src/hk/services/socket.cpp
    src/hk/services/sm.cpp
    src/hk/services/pm.cpp
    src/hk/services/nv.cpp
    src/hk/services/lm.cpp

    src/hk/sf/sf.cpp

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
add_library(LibHakkunForModule OBJECT
    src/hk/diag/diag.cpp
    src/hk/diag/ipclogger.cpp
    src/hk/diag/ResultName.cpp
    
    src/hk/init/module.cpp
)

apply_config(LibHakkun)
apply_config(LibHakkunForModule)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
