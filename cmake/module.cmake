include(config/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/apply_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/generate_exefs.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/addons.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/module_config.cmake)

set(CMAKE_EXECUTABLE_SUFFIX ".nss")
set(ROOTDIR ${CMAKE_CURRENT_SOURCE_DIR})

target_link_options(${PROJECT_NAME} PRIVATE -T${MISC_LINKER_SCRIPT})

apply_config(${PROJECT_NAME})

if (MODULE_BINARY)
    if (MODULE_BINARY STREQUAL "rtld")
        message(FATAL_ERROR "Hakkun cannot be used in place of rtld")
    endif()

    if (HAKKUN_TARGET STREQUAL MODULE_DLL)
        message(FATAL_ERROR "cannot specify MODULE_BINARY with MODULE_DLL")
    endif()
endif()

add_subdirectory(sys/hakkun)
add_to_visibility(${PROJECT_SOURCE_DIR}/exported.txt)
add_to_visibility(${PROJECT_SOURCE_DIR}/sys/data/exported_syms_module.txt)
if (HAKKUN_TARGET STREQUAL MODULE_STANDALONE)
    if (IS_32_BIT)
        set(REGISTER_WIDTH 32)
    else()
        set(REGISTER_WIDTH 64)
    endif()
    add_to_visibility(${PROJECT_SOURCE_DIR}/sys/data/exported_syms_module_standalone_${REGISTER_WIDTH}.txt)
endif()
apply_module_config(${PROJECT_NAME} TRUE __module_entry__)
target_link_libraries(${PROJECT_NAME} PRIVATE LibHakkunForModule)

generate_exefs()

if (TARGET_IS_STATIC)
    include(sys/cmake/rtld_dummy.cmake)

    add_rtld_dummy()
    add_dependencies(${PROJECT_NAME} rtld)
endif()

if (HAKKUN_TARGET STREQUAL MODULE_STANDALONE)
    include(sys/cmake/rtld_standalone.cmake)

    add_rtld_standalone()
    add_dependencies(${PROJECT_NAME} rtld)
endif()

enable_addons(${PROJECT_NAME})
