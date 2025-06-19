cmake_minimum_required(VERSION 3.16)
project(sail)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(sail src/main.cpp src/parser.cpp src/config.cpp src/fakelib.cpp)

target_compile_options(sail PRIVATE -O3 -ffast-math -flto -Wno-format-security)
target_link_options(sail PRIVATE -flto)
