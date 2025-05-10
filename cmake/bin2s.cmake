cmake_minimum_required(VERSION 3.13)

function(embed_file PROJECT_NAME INPUT_FILE FILENAME ALIGN IS_DATA)
  set(SPATH ${CMAKE_CURRENT_BINARY_DIR}/${FILENAME}_bin2s.S)
  set(HEADER_FILE ${CMAKE_CURRENT_BINARY_DIR}/embed_${FILENAME}.h)
  
  get_filename_component(FILENAME ${INPUT_FILE} NAME)
  
  string(REGEX REPLACE "[^a-zA-Z0-9_]" "_" SYMBOL_NAME ${FILENAME})
  string(REGEX REPLACE "^[0-9]" "_\\0" SYMBOL_NAME ${SYMBOL_NAME})
  
  add_custom_command(
    OUTPUT ${SPATH} ${HEADER_FILE}
    COMMAND ${CMAKE_COMMAND}
      -DINPUT_FILE=${INPUT_FILE}
      -DOUTPUT_ASM_FILE=${SPATH}
      -DOUTPUT_HEADER_FILE=${HEADER_FILE}
      -DSYMBOL_NAME=${SYMBOL_NAME}
      -DALIGNMENT=${ALIGN}
      -DIS_DATA=${IS_DATA}
      -P ${ROOTDIR}/sys/cmake/impl/bin2s.cmake
    DEPENDS ${INPUT_FILE}
    VERBATIM
  )
  
  target_sources(${PROJECT_NAME} PRIVATE ${SPATH})
  
  get_filename_component(HEADER_DIR ${HEADER_FILE} DIRECTORY)
  target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})
endfunction()
