function(apply_config project)
    foreach(item IN LISTS LLDFLAGS)
        list(APPEND LLDFLAGS_WL "-Wl,${item}")
    endforeach()
    target_link_options(${project} PRIVATE ${LLDFLAGS_WL})

    if (TARGET_IS_STATIC)
        target_compile_definitions(${project} PRIVATE TARGET_IS_STATIC)
    endif()

    if (SDK_PAST_1900)
        target_compile_definitions(${project} PRIVATE __RTLD_PAST_19XX__)
    endif()

    if (NOT USE_SAIL)
        target_compile_definitions(${project} PRIVATE HK_DISABLE_SAIL)
    endif()

    target_compile_definitions(${project} PRIVATE NNSDK HK_HOOK_TRAMPOLINE_POOL_SIZE=${TRAMPOLINE_POOL_SIZE} MODULE_NAME=${MODULE_NAME})

    target_include_directories(${project} PRIVATE ${INCLUDES})
    target_compile_definitions(${project} PRIVATE ${DEFINITIONS})

    if (BAKE_SYMBOLS)
        target_compile_definitions(${project} PRIVATE HK_USE_PRECALCULATED_SYMBOL_DB_HASHES)
    endif()

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
    
    target_compile_definitions(${project} PRIVATE HK_TITLE_ID=${TITLE_ID})
endfunction()
