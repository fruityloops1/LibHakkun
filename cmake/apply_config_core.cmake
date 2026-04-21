function(apply_config_core project)
    foreach(item IN LISTS LLDFLAGS)
        list(APPEND LLDFLAGS_WL "-Wl,${item}")
    endforeach()
    target_link_options(${project} PRIVATE ${LLDFLAGS_WL})

    if (USE_ADVANCED_RESULT)
        target_compile_definitions(${project} PRIVATE HK_RESULT_ADVANCED=1)
    else()
        target_compile_definitions(${project} PRIVATE HK_RESULT_ADVANCED=0)
    endif()

    target_include_directories(${project} PRIVATE ${INCLUDES})
    target_compile_definitions(${project} PRIVATE ${DEFINITIONS})

    if(CMAKE_BUILD_TYPE STREQUAL Release)
        set(OPTIMIZE_OPTIONS ${OPTIMIZE_OPTIONS_RELEASE})
    else()
        set(OPTIMIZE_OPTIONS ${OPTIMIZE_OPTIONS_DEBUG})
    endif()

    target_compile_options(${project} PRIVATE
        $<$<COMPILE_LANGUAGE:ASM>:${ASM_OPTIONS}>
    )
    target_compile_options(${project} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:${OPTIMIZE_OPTIONS} ${WARN_OPTIONS} ${C_OPTIONS}>
    )
    target_compile_options(${project} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:${OPTIMIZE_OPTIONS} ${WARN_OPTIONS} ${C_OPTIONS} ${CXX_OPTIONS}>
    )
    
    target_link_options(${project} PRIVATE ${LINKFLAGS} ${OPTIMIZE_OPTIONS})
endfunction()
