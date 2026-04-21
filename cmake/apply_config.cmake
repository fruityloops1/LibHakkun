include(${CMAKE_CURRENT_LIST_DIR}/apply_config_core.cmake)

function(apply_config project)
    apply_config_core(${project})

    if (TARGET_IS_STATIC)
        target_compile_definitions(${project} PRIVATE TARGET_IS_STATIC)

        if (HAKKUN_STANDALONE)
            message(FATAL_ERROR "HAKKUN_STANDALONE cannot be used with target")
        endif()
    endif()

    if (HAKKUN_STANDALONE)
        target_compile_definitions(${project} PRIVATE HK_STANDALONE)
    endif()

    if (SDK_PAST_1900)
        target_compile_definitions(${project} PRIVATE __RTLD_PAST_19XX__)
    endif()

    if (NOT USE_SAIL)
        target_compile_definitions(${project} PRIVATE HK_DISABLE_SAIL)
    endif()

    target_compile_definitions(${project} PRIVATE NNSDK HK_HOOK_TRAMPOLINE_POOL_SIZE=${TRAMPOLINE_POOL_SIZE} MODULE_NAME=${MODULE_NAME})

    if (BAKE_SYMBOLS)
        target_compile_definitions(${project} PRIVATE HK_USE_PRECALCULATED_SYMBOL_DB_HASHES)
    endif()
    
    target_compile_definitions(${project} PRIVATE HK_TITLE_ID=${TITLE_ID})
endfunction()
