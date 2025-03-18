function(generate_exefs)
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
            COMMAND ${PROJECT_SOURCE_DIR}/sys/tools/elf2nso.py ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}.baked ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nso -c
        )
    else()
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "-- Generating ${PROJECT_NAME}.nso"
            COMMAND ${PROJECT_SOURCE_DIR}/sys/tools/elf2nso.py ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.nso -c
        )
    endif()
endfunction()
