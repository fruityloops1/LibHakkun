file(TO_CMAKE_PATH "$ENV{BIN2S}" BIN2S)

if(NOT EXISTS ${BIN2S})
    if(NOT IS_DIRECTORY ${DEVKITPRO})
        message(FATAL_ERROR "Please install devkitA64 or set BIN2S in your environment.")
    else()
        set(BIN2S ${DEVKITPRO}/tools/bin/bin2s)
    endif()
endif()

function(embed_file project filepath filename)
    set(SPATH ${CMAKE_CURRENT_BINARY_DIR}/${filename}_shader.S)

    add_custom_command(OUTPUT ${SPATH} PRE_BUILD
        COMMAND ${BIN2S} ${filepath} --header ${CMAKE_CURRENT_BINARY_DIR}/embed_${filename}.h > ${SPATH} && sed -i -f ${ROOTDIR}/sys/data/replace_bin2s.sed ${SPATH}
        DEPENDS ${filepath}
        )
    set_source_files_properties(${SPATH} PROPERTIES GENERATED TRUE)
    target_sources(${project} PRIVATE ${SPATH})
    target_include_directories(${project} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
