set(SAIL_BIN ${CMAKE_CURRENT_SOURCE_DIR}/sys/hakkun/sail/build/sail)
set(SAIL_LIBS
    ${CMAKE_CURRENT_BINARY_DIR}/symboldb.o
    ${CMAKE_CURRENT_BINARY_DIR}/fakesymbols.so
    )

include(config/config.cmake)
include(sys/cmake/watch.cmake)

function (usesail lib)
    if (USE_SAIL)
        target_link_options(${lib} PRIVATE ${SAIL_LIBS})

        if(NOT EXISTS ${SAIL_BIN})
            message(FATAL_ERROR "Sail binary not found! Did you run setup_sail.py?")
        endif()

        file(GLOB_RECURSE SYM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/syms/*.sym)

        set(SYMDEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym")
        set(SYMDEPENDS "${SYMDEPENDS};${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym")

        foreach(item IN LISTS SYM_FILES)
            set(SYMDEPENDS "${SYMDEPENDS};${item}")
            watch(${lib} ${item})
        endforeach()

        watch(${lib} ${SYMDEPENDS})

        add_custom_command(TARGET ${lib} PRE_LINK
            COMMAND ${SAIL_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym ${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym ${CMAKE_CURRENT_SOURCE_DIR}/syms ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_ASM_COMPILER} $<IF:$<BOOL:${IS_32_BIT}>,1,0>
        )
    endif()
endfunction()
