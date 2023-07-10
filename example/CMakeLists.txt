cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
project(PalexExample)

set(PROJECT_SOURCES
    src/example_parser.cpp
)
set(GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleLexer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleLexer.h
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleParser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleParser.h
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleTypes.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/ExampleTypes.h
    ${CMAKE_CURRENT_BINARY_DIR}/utf8.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/utf8.h
)
add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND palex ../Example.palex 
                -output-path ${CMAKE_CURRENT_BINARY_DIR} -util-path ${CMAKE_CURRENT_BINARY_DIR}
                -lang C++ -lookahead 1 -parser-type LALR 
                --fallback --lexer --util --parser
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Example.palex 
)
add_executable(example_parser ${PROJECT_SOURCES} ${GENERATED_FILES})
target_include_directories(example_parser PRIVATE ${CMAKE_CURRENT_BINARY_DIR})