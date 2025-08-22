file(TO_CMAKE_PATH "$ENV{SWITCHTOOLS}" SWITCHTOOLS)
file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)

include(config/config.cmake)
include(sys/cmake/apply_config.cmake)
include(sys/cmake/generate_exefs.cmake)
include(sys/cmake/addons.cmake)
include(sys/cmake/module_config.cmake)

if(NOT IS_DIRECTORY ${SWITCHTOOLS})
    if(NOT IS_DIRECTORY ${DEVKITPRO})
        message(FATAL_ERROR "Please install devkitA64 or set SWITCHTOOLS in your environment.")
    else()
        set(SWITCHTOOLS ${DEVKITPRO}/tools/bin)
    endif()
endif()

if (MODULE_BINARY STREQUAL "rtld")
    message(FATAL_ERROR "Hakkun cannot be used in place of rtld")
endif()

set(CMAKE_EXECUTABLE_SUFFIX ".nss")
set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(${PROJECT_NAME} PRIVATE -T${MISC_LINKER_SCRIPT})
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--export-dynamic-symbol=_ZN2nn2ro6detail15g_pAutoLoadListE)

add_subdirectory(sys/hakkun)
apply_module_config(${PROJECT_NAME} TRUE __module_entry__)
target_link_libraries(${PROJECT_NAME} PRIVATE LibHakkunForModule)

generate_exefs()

if (TARGET_IS_STATIC)
    include(sys/cmake/rtld_dummy.cmake)

    add_rtld_dummy()
endif()

if (HAKKUN_STANDALONE)
    include(sys/cmake/rtld_standalone.cmake)

    add_rtld_standalone()
endif()

enable_addons(${PROJECT_NAME})
