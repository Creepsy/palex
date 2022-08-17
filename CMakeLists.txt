cmake_minimum_required(VERSION 3.23.3)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

project(palex)

set(PROJECT_SOURCES
    src/util/encoding.cpp
)

set(PROJECT_HEADERS
    src/util/encoding.h
)

add_executable(palex src/palex.cpp ${PROJECT_SOURCES} ${PROJECT_HEADERS})