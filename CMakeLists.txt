cmake_minimum_required(VERSION 3.23.3)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

project(palex)

include(CTest)

add_subdirectory(src)
add_subdirectory(tests)
add_test(NAME UtilTest COMMAND util_test)
add_test(NAME LexerGeneratorTest COMMAND lexer_generator_test)