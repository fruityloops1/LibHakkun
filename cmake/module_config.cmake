include(config/config.cmake)
include(sys/cmake/apply_config.cmake)
include(sys/cmake/generate_exefs.cmake)
include(sys/cmake/addons.cmake)
include(sys/cmake/watch.cmake)
include(sys/cmake/visibility.cmake)

if (IS_32_BIT)
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/link.armv7a.ld")
else()
    set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/link.aarch64.ld")
endif()
set(MISC_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/misc.ld")

watch(${PROJECT_NAME} "${LINKER_SCRIPT};${MISC_LINKER_SCRIPT}")

function(apply_module_config module useLinkerScript init)
    set(VERSION_SCRIPT_FILE "${CMAKE_BINARY_DIR}/${module}_visibility.txt")

    if (useLinkerScript)
        target_link_options(${module} PRIVATE -T${LINKER_SCRIPT})
    endif()
    write_visibility_script(${VERSION_SCRIPT_FILE} ${module})
    target_link_options(${module} PRIVATE -Wl,-init=${init} -Wl,--pie)

    if (EXISTS ${VERSION_SCRIPT_FILE})
        target_link_options(${module} PRIVATE -Wl,--version-script=${VERSION_SCRIPT_FILE})
    endif()

    apply_config(${module})

    target_link_libraries(${module} PRIVATE LibHakkun)
    target_include_directories(${module} PRIVATE sys/hakkun/include)

    clear_visibility()
endfunction()
