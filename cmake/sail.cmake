set(SAIL_BIN ${CMAKE_CURRENT_SOURCE_DIR}/sys/hakkun/sail/build/sail)
set(SAIL_LIBS
    ${CMAKE_CURRENT_BINARY_DIR}/symboldb.o
    ${CMAKE_CURRENT_BINARY_DIR}/datablocks.o
    ${CMAKE_CURRENT_BINARY_DIR}/fakesymbols.so
    )

include(config/config.cmake)
include(sys/cmake/watch.cmake)
set(SAIL_REVISION D)

function (usesail lib)
    if (USE_SAIL)
        target_link_options(${lib} PRIVATE ${SAIL_LIBS})

        if(NOT EXISTS ${SAIL_BIN})
            message(WARNING "Sail binary not found! Running setup_sail.py")
            execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/sys/tools/setup_sail.py
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE result
            )
        endif()

        file(GLOB_RECURSE SYM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/syms/*.sym ${CMAKE_CURRENT_SOURCE_DIR}/sys/addons/*/syms/*.sym)

        set(SYMDEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym")
        set(SYMDEPENDS "${SYMDEPENDS};${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym")

        foreach(item IN LISTS SYM_FILES)
            set(SYMDEPENDS "${SYMDEPENDS};${item}")
            watch(${lib} ${item})
        endforeach()

        watch(${lib} ${SYMDEPENDS})
        
        set(SAIL_CMD ${SAIL_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym ${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_ASM_COMPILER} $<IF:$<BOOL:${IS_32_BIT}>,1,0> ${SAIL_REVISION} ${CMAKE_CURRENT_SOURCE_DIR}/syms)
        
        file(GLOB_RECURSE ADDONS_SYMS_EMPTY_TEST ${CMAKE_CURRENT_SOURCE_DIR}/sys/addons/*/syms/*.sym)
        if (ADDONS_SYMS_EMPTY_TEST)
            set(SAIL_CMD ${SAIL_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/sys/addons/*/syms)
        endif()
        
        add_custom_command(TARGET ${lib} PRE_LINK
            COMMAND ${SAIL_CMD}
        )
    endif()
endfunction()
