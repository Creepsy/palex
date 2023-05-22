cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_compile_definitions(PALEX_VERSION="1.0.0")

project(palex)

include(CTest)
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()