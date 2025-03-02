file(TO_CMAKE_PATH "$ENV{SWITCHTOOLS}" SWITCHTOOLS)
file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)

include(config/config.cmake)
include(sys/cmake/apply_config.cmake)
include(sys/cmake/generate_exefs.cmake)
include(sys/cmake/addons.cmake)

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

include(sys/cmake/watch.cmake)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/link.ld")
set(MISC_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/sys/data/misc.ld")
watch(${PROJECT_NAME} "${LINKER_SCRIPT};${MISC_LINKER_SCRIPT}")
target_link_options(${PROJECT_NAME} PRIVATE -T${LINKER_SCRIPT} -T${MISC_LINKER_SCRIPT})
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-init=__module_entry__ -Wl,--pie -Wl,--export-dynamic-symbol=_ZN2nn2ro6detail15g_pAutoLoadListE)

apply_config(${PROJECT_NAME})

add_subdirectory(sys/hakkun)
target_link_libraries(${PROJECT_NAME} PRIVATE LibHakkun)
target_include_directories(${PROJECT_NAME} PRIVATE sys/hakkun/include)

generate_exefs()

if (TARGET_IS_STATIC)
    include(sys/cmake/rtld_dummy.cmake)

    add_rtld_dummy()
endif()

enable_addons()
