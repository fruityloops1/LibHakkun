function(add_rtld_dummy)
    add_executable(rtld
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/DummyRtld.cpp
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/hk/init/module.S
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/hk/svc/api.aarch64.S
    )
    
    target_include_directories(rtld PRIVATE
        ${PROJECT_SOURCE_DIR}/sys/hakkun/include
    )
    
    target_link_options(rtld PRIVATE -T${LINKER_SCRIPT} -Wl,--export-dynamic -Wl,--pie)
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Generating rtld.nso"
        COMMAND ${PROJECT_SOURCE_DIR}/sys/tools/elf2nso.py ${CMAKE_CURRENT_BINARY_DIR}/rtld${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/rtld.nso
    )
endfunction()