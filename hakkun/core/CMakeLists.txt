cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    src/hk/diag/diag.cpp
    src/hk/diag/ResultName.cpp
    )

add_library(LibHakkunCore ${SOURCES})
target_include_directories(LibHakkunCore PUBLIC include)
