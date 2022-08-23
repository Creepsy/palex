cmake_minimum_required(VERSION 3.23.3)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

project(palex)

set(PROJECT_SOURCES
    src/util/encoding.cpp
    src/util/palex_except.cpp

    src/lexer_generator/LexerRuleLexer.cpp
    src/lexer_generator/LexerRuleParser.cpp
)

add_executable(palex src/palex.cpp ${PROJECT_SOURCES})
target_include_directories(palex PRIVATE src/)