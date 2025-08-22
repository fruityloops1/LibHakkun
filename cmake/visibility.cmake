include(sys/cmake/watch.cmake)

set_property(GLOBAL PROPERTY VISIBILITY_SYMBOLS_LIST "")

function(add_to_visibility file)
    file(READ "${file}" file_content)
    
    string(REPLACE "\r\n" "\n" file_content "${file_content}") 
    string(REPLACE "\r" "\n" file_content "${file_content}")
    string(REGEX REPLACE "\n$" "" file_content "${file_content}")
    
    if(file_content)
        string(REPLACE "\n" ";" lines "${file_content}")
        
        get_property(current_symbols GLOBAL PROPERTY VISIBILITY_SYMBOLS_LIST)
        
        foreach(line ${lines})
            string(STRIP "${line}" line)
            
            if(line AND NOT line MATCHES "^#")
                list(APPEND current_symbols "${line}")
            endif()
        endforeach()
        
        set_property(GLOBAL PROPERTY VISIBILITY_SYMBOLS_LIST "${current_symbols}")
    endif()
endfunction()

function(write_visibility_script output_file)
    get_property(symbols GLOBAL PROPERTY VISIBILITY_SYMBOLS_LIST)
    
    list(REMOVE_DUPLICATES symbols)
    
    set(script_content "{\n")
    set(script_content "${script_content}  global:\n")
    #
    foreach(symbol ${symbols})
        set(script_content "${script_content}    ${symbol};\n")
    endforeach()
    
    set(script_content "${script_content}};\n")
    
    file(WRITE "${output_file}" "${script_content}")
endfunction()

function(clear_visibility)
    set_property(GLOBAL PROPERTY VISIBILITY_SYMBOLS_LIST "")
endfunction()