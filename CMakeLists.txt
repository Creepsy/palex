cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

project(palex)

include(CTest)
include_directories(${CMAKE_SOURCE_DIR}/vendor ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(tests)

enable_testing()