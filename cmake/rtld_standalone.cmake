include(sys/cmake/module_config.cmake)
include(sys/cmake/visibility.cmake)

function(add_rtld_standalone)
    add_executable(rtld
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/standalone/entry.S
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/standalone/diag.cpp
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/standalone/main.cpp
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/standalone/module.cpp
    )
    
    target_include_directories(rtld PRIVATE
        ${PROJECT_SOURCE_DIR}/sys/hakkun/include
    )
    
    apply_module_config(rtld TRUE rtldInit)
    target_link_options(rtld PRIVATE -Wl,--export-dynamic)
    
    add_library(SdkImportsFakeLib SHARED
        ${PROJECT_SOURCE_DIR}/sys/hakkun/src/rtld/standalone/SdkImportsFakeLib.cpp
    )   
    target_link_options(SdkImportsFakeLib PRIVATE -Wl,--export-dynamic)
    target_link_options(SdkImportsFakeLib PRIVATE -Wl,-fini=rtldFini)
    add_to_visibility(${PROJECT_SOURCE_DIR}/sys/data/exported_syms_rtld_fakelib.txt)
    apply_module_config(SdkImportsFakeLib FALSE _start)

    target_link_libraries(rtld PRIVATE SdkImportsFakeLib)

    add_custom_command(TARGET rtld POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "-- Generating rtld.nso"
        COMMAND ${PROJECT_SOURCE_DIR}/sys/tools/elf2nso.py ${CMAKE_CURRENT_BINARY_DIR}/rtld${CMAKE_EXECUTABLE_SUFFIX} ${CMAKE_CURRENT_BINARY_DIR}/rtld.nso -c
    )
endfunction()