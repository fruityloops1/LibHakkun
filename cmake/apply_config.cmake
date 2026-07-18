include(${CMAKE_CURRENT_LIST_DIR}/apply_config_core.cmake)

function(apply_config project)
    apply_config_core(${project})

    target_compile_definitions(${project} PRIVATE HK_TARGET_MODULE=0 HK_TARGET_MODULE_STANDALONE=1 HK_TARGET_MODULE_DLL=2)
    if (NOT HAKKUN_TARGET)
        if (HAKKUN_STANDALONE)
            message(WARNING "HAKKUN_TARGET not set; setting to MODULE_STANDALONE because HAKKUN_STANDALONE is set, please set HAKKUN_TARGET")
            set(HAKKUN_TARGET MODULE_STANDALONE)
        else()
            message(WARNING "HAKKUN_TARGET not set; defaulting to MODULE, please set HAKKUN_TARGET")
            set(HAKKUN_TARGET MODULE)
        endif()

        set(HAKKUN_TARGET ${HAKKUN_TARGET} PARENT_SCOPE)
    endif()

    target_compile_definitions(${project} PRIVATE HK_HOMEBREW_NONE=0 HK_HOMEBREW_HBLOADER=1)
    if (NOT HOMEBREW_TYPE)
        set(HOMEBREW_TYPE NONE)
        set(HOMEBREW_TYPE ${HOMEBREW_TYPE} PARENT_SCOPE)
    endif()

    if (HAKKUN_TARGET STREQUAL MODULE)
        target_compile_definitions(${project} PRIVATE HK_TARGET=HK_TARGET_MODULE)
    elseif (HAKKUN_TARGET STREQUAL MODULE_STANDALONE)
        target_compile_definitions(${project} PRIVATE HK_TARGET=HK_TARGET_MODULE_STANDALONE)
    elseif (HAKKUN_TARGET STREQUAL MODULE_DLL)
        target_compile_definitions(${project} PRIVATE HK_TARGET=HK_TARGET_MODULE_DLL)

        if (USE_SAIL)
            message(FATAL_ERROR "MODULE_DLL cannot be used with sail")
        endif()
    else()
        message(FATAL_ERROR "invalid HAKKUN_TARGET: ${HAKKUN_TARGET}")
    endif()

    if (HOMEBREW_TYPE STREQUAL NONE)
        target_compile_definitions(${project} PRIVATE HK_HOMEBREW_TYPE=HK_HOMEBREW_NONE)
    elseif (HOMEBREW_TYPE STREQUAL HBLOADER)
        target_compile_definitions(${project} PRIVATE HK_HOMEBREW_TYPE=HK_HOMEBREW_HBLOADER)

        if (NOT HAKKUN_TARGET STREQUAL MODULE_DLL)
            message(FATAL_ERROR "can only use HBLOADER with MODULE_DLL")
        endif()
    else()
        message(FATAL_ERROR "invalid HOMEBREW_TYPE: ${HOMEBREW_TYPE}")
    endif()

    if (TARGET_IS_STATIC)
        target_compile_definitions(${project} PRIVATE TARGET_IS_STATIC)

        if (HAKKUN_TARGET STREQUAL MODULE_STANDALONE)
            message(FATAL_ERROR "MODULE_STANDALONE cannot be used with target")
        endif()
    endif()

    if (SDK_PAST_1900)
        target_compile_definitions(${project} PRIVATE __RTLD_PAST_19XX__)
    endif()

    if (NOT USE_SAIL)
        target_compile_definitions(${project} PRIVATE HK_DISABLE_SAIL)
    endif()
    
    if(NOT TRAMPOLINE_LEVEL)
        message(WARNING "TRAMPOLINE_LEVEL not set; using 1")
        set(TRAMPOLINE_LEVEL 1)
        set(TRAMPOLINE_LEVEL ${TRAMPOLINE_LEVEL} PARENT_SCOPE)
    endif()
    
    target_compile_definitions(${project} PRIVATE NNSDK HK_HOOK_TRAMPOLINE_LEVEL=${TRAMPOLINE_LEVEL} MODULE_NAME=${MODULE_NAME})

    if (TRAMPOLINE_POOL_SIZE)
        message(WARNING "TRAMPOLINE_POOL_SIZE is deprecated; remove it from config.cmake")
    endif()

    if (BAKE_SYMBOLS)
        target_compile_definitions(${project} PRIVATE HK_USE_PRECALCULATED_SYMBOL_DB_HASHES)
    endif()
    
    if (TITLE_ID AND NOT HAKKUN_TARGET STREQUAL MODULE_DLL)
        target_compile_definitions(${project} PRIVATE HK_TITLE_ID=${TITLE_ID})
    endif()
endfunction()
