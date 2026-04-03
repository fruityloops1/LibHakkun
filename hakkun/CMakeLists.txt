cmake_minimum_required(VERSION 3.16)

include(${PROJECT_SOURCE_DIR}/config/config.cmake)
include(${CMAKE_SOURCE_DIR}/sys/cmake/apply_config.cmake)

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

    src/hk/services/am.cpp
    src/hk/services/apm.cpp
    src/hk/services/fsp.cpp
    src/hk/services/ldr.cpp
    src/hk/services/lm.cpp
    src/hk/services/nv.cpp
    src/hk/services/pm.cpp
    src/hk/services/settings.cpp
    src/hk/services/socket.cpp
    src/hk/services/sm.cpp
    src/hk/services/vi.cpp

    src/hk/sf/sf.cpp
    src/hk/sf/tipc.cpp

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

set(SOURCES
    src/hk/diag/diag.cpp
    src/hk/diag/ipclogger.cpp
    src/hk/diag/ResultName.cpp
    )

if (NOT HAKKUN_CUSTOM_ENTRY)
    set(SOURCES ${SOURCES}
        src/hk/init/module.cpp
        )
endif()

add_library(LibHakkunForModule OBJECT ${SOURCES})

apply_config(LibHakkun)
apply_config(LibHakkunForModule)

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
