set(SAIL_BIN ${CMAKE_CURRENT_SOURCE_DIR}/sys/sail/build/sail)
set(SAIL_LIBS
    ${CMAKE_CURRENT_BINARY_DIR}/symboldb.o
    ${CMAKE_CURRENT_BINARY_DIR}/datablocks.o
    ${CMAKE_CURRENT_BINARY_DIR}/fakesymbols.so
    )

include(config/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/watch.cmake)
set(SAIL_REVISION K)

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

        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym)
            message(WARNING "ModuleList.sym is deprecated; ignoring file")
        endif()

        set(SYMDEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym")

        foreach(item IN LISTS SYM_FILES)
            set(SYMDEPENDS "${SYMDEPENDS};${item}")
            watch(${lib} ${item})
        endforeach()

        watch(${lib} ${SYMDEPENDS})
        
        set(SAIL_CMD ${SAIL_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/config/ModuleList.sym ${CMAKE_CURRENT_SOURCE_DIR}/config/VersionList.sym ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CXX_COMPILER} $<IF:$<BOOL:${IS_32_BIT}>,1,0> ${SAIL_REVISION} ${CMAKE_CURRENT_SOURCE_DIR}/syms)
        
        file(GLOB_RECURSE ADDONS_SYMS_EMPTY_TEST ${CMAKE_CURRENT_SOURCE_DIR}/sys/addons/*/syms/*.sym)
        if (ADDONS_SYMS_EMPTY_TEST)
            # Expand the glob ourselves rather than passing a literal pattern.
            # On POSIX, /bin/sh expands the wildcard before sail sees it, so
            # the original `.../sys/addons/*/syms` form happens to work. On
            # Windows, add_custom_command(COMMAND ...) invokes the command
            # directly (no shell glob), and sail receives the literal string
            # "...sys/addons/*/syms" which doesn't exist — recursive_directory_iterator
            # then throws and sail aborts. Expand at configure time so both
            # platforms hand sail real directory paths.
            file(GLOB ADDONS_SYM_DIRS LIST_DIRECTORIES TRUE
                 ${CMAKE_CURRENT_SOURCE_DIR}/sys/addons/*/syms)
            foreach (d IN LISTS ADDONS_SYM_DIRS)
                if (IS_DIRECTORY ${d})
                    set(SAIL_CMD ${SAIL_CMD} ${d})
                endif()
            endforeach()
        endif()
        
        add_custom_command(TARGET ${lib} PRE_LINK
            COMMAND ${SAIL_CMD}
        )
    endif()
endfunction()
