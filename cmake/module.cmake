file(TO_CMAKE_PATH "$ENV{SWITCHTOOLS}" SWITCHTOOLS)
file(TO_CMAKE_PATH "$ENV{DEVKITPRO}" DEVKITPRO)

include(config/config.cmake)

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

foreach(item IN LISTS LLDFLAGS)
    list(APPEND LLDFLAGS_WL "-Wl,${item}")
endforeach()
target_link_options(${PROJECT_NAME} PRIVATE ${LLDFLAGS_WL})

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set(OPTIMIZE_OPTIONS ${OPTIMIZE_OPTIONS_RELEASE})
else()
    set(OPTIMIZE_OPTIONS ${OPTIMIZE_OPTIONS_DEBUG})
endif()

target_link_options(${PROJECT_NAME} PRIVATE ${LINKFLAGS} ${OPTIMIZE_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE -Wl,-init=__module_entry__ -Wl,--pie -Wl,--export-dynamic-symbol=_ZN2nn2ro6detail15g_pAutoLoadListE)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:${ASM_OPTIONS}>
)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${OPTIMIZE_OPTIONS} ${WARN_OPTIONS} ${C_OPTIONS}>
)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${OPTIMIZE_OPTIONS} ${WARN_OPTIONS} ${C_OPTIONS} ${CXX_OPTIONS}>
)

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${DEFINITIONS})
target_compile_definitions(${PROJECT_NAME} PRIVATE NNSDK)
target_compile_definitions(${PROJECT_NAME} PRIVATE HK_HOOK_TRAMPOLINE_POOL_SIZE=${TRAMPOLINE_POOL_SIZE})

if (SDK_PAST_1900)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __RTLD_PAST_19XX__)
endif()
if (NOT USE_SAIL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HK_DISABLE_SAIL)
endif()

add_subdirectory(sys/hakkun)
target_link_libraries(${PROJECT_NAME} PRIVATE LibHakkun)
target_include_directories(${PROJECT_NAME} PRIVATE sys/hakkun/include)

configure_file(${PROJECT_SOURCE_DIR}/config/npdm.json ${CMAKE_CURRENT_BINARY_DIR}/npdm.json)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "-- Generating main.npdm"
    COMMAND ${SWITCHTOOLS}/npdmtool ${CMAKE_CURRENT_BINARY_DIR}/npdm.json ${CMAKE_CURRENT_BINARY_DIR}/main.npdm 2>> ${CMAKE_CURRENT_BINARY_DIR}/npdmtool.log
)

if (USE_SAIL AND BAKE_SYMBOLS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "-- Generating ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}.baked"
    COMMAND python ${CMAKE_SOURCE_DIR}/sys/tools/bake_hashes.py ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Generating ${PROJECT_NAME}.nso"
        COMMAND ${SWITCHTOOLS}/elf2nso ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}.baked ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nso
    )
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Generating ${PROJECT_NAME}.nso"
        COMMAND ${SWITCHTOOLS}/elf2nso ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nso
    )
endif()

if (TARGET_IS_STATIC)
    add_executable(rtld
    ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/DummyRtld.cpp
    ${PROJECT_SOURCE_DIR}/sys/hakkun/src/hk/init/module.S
    ${PROJECT_SOURCE_DIR}/sys/hakkun/src/hk/svc/api.S
    )

    target_include_directories(rtld PRIVATE
        ${PROJECT_SOURCE_DIR}/sys/hakkun/include
    )

    target_link_options(rtld PRIVATE -T${LINKER_SCRIPT} -T${MISC_LINKER_SCRIPT} -Wl,--export-dynamic -Wl,--pie)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Generating rtld.nso"
        COMMAND ${SWITCHTOOLS}/elf2nso ${CMAKE_CURRENT_BINARY_DIR}/rtld${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/rtld.nso
    )
endif()
