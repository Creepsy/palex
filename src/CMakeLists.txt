set(PROJECT_SOURCES
    util/unicode.cpp
    util/palex_except.cpp

    util/regex/regex_ast.cpp
    util/regex/RegexParser.cpp

    lexer_generator/LexerRuleLexer.cpp
    lexer_generator/LexerRuleParser.cpp
)

add_library(palex_objects ${PROJECT_SOURCES})
target_include_directories(palex_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(palex palex.cpp)
target_include_directories(palex PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(palex palex_objects)