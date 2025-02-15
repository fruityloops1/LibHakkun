function(watch lib)
    set_target_properties(
        ${lib}
        PROPERTIES
        LINK_DEPENDS
        "${ARGN}"
    )
endfunction()
