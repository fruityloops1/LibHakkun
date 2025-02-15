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

if(CMAKE_BUILD_TYPE STREQUAL Release)
    target_compile_options(LibHakkun PRIVATE ${OPTIMIZE_OPTIONS_RELEASE})
else()
    target_compile_options(LibHakkun PRIVATE ${OPTIMIZE_OPTIONS_DEBUG})
endif()

if (TARGET_IS_STATIC)
    target_compile_definitions(LibHakkun PRIVATE TARGET_IS_STATIC)
endif()

if (SDK_PAST_1900)
    target_compile_definitions(LibHakkun PRIVATE __RTLD_PAST_19XX__)
endif()

target_compile_options(LibHakkun PRIVATE ${WARN_OPTIONS})
target_compile_definitions(LibHakkun PRIVATE NNSDK MODULE_NAME=${MODULE_NAME} HK_HOOK_TRAMPOLINE_POOL_SIZE=${TRAMPOLINE_POOL_SIZE})

if (BAKE_SYMBOLS)
    target_compile_definitions(LibHakkun PRIVATE HK_USE_PRECALCULATED_SYMBOL_DB_HASHES)
endif()

set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../)
