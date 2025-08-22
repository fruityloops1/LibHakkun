include(config/config.cmake)
include(sys/cmake/apply_config.cmake)
include(sys/cmake/generate_exefs.cmake)
include(sys/cmake/addons.cmake)
include(sys/cmake/watch.cmake)

set(VERSION_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/visibility.txt")
set(USER_VERSION_SCRIPT "${CMAKE_SOURCE_DIR}/config/visibility.txt")
if (EXISTS ${USER_VERSION_SCRIPT})
    set(USER_VERSION_SCRIPT_ARG "-Wl,--version-script=${USER_VERSION_SCRIPT}")
else()
    set(USER_VERSION_SCRIPT_ARG "")
endif()

if (IS_32_BIT)
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/link.armv7a.ld")
else()
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/link.aarch64.ld")
endif()
set(MISC_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/misc.ld")

watch(${PROJECT_NAME} "${LINKER_SCRIPT};${MISC_LINKER_SCRIPT}")

function(apply_module_config module useLinkerScript init)

    if (useLinkerScript)
        target_link_options(${module} PRIVATE -T${LINKER_SCRIPT})
    endif()
    target_link_options(${module} PRIVATE -Wl,-init=${init} -Wl,--pie -Wl,--version-script=${VERSION_SCRIPT} ${USER_VERSION_SCRIPT_ARG})
    apply_config(${module})

    target_link_libraries(${module} PRIVATE LibHakkun)
    target_include_directories(${module} PRIVATE sys/hakkun/include)
endfunction()
