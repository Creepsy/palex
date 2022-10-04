set(PROJECT_SOURCES
    util/unicode.cpp
    util/palex_except.cpp

    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/LexerRuleLexer.cpp
    lexer_generator/LexerRuleParser.cpp
    lexer_generator/lexer_automaton.cpp
    lexer_generator/validation.cpp
)

set(PROJECT_HEADERS
    util/Automaton.h
    util/unicode.h
    util/palex_except.h

    regex/character_classes.h
    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/LexerRuleLexer.h
    lexer_generator/LexerRuleParser.h
    lexer_generator/lexer_automaton.h
    lexer_generator/validation.h
)

set(PROJECT_INLINE_FILES
    util/Automaton.ipp

    regex/RegexParser.ipp
)

include(templates/all_template_headers.cmake)

add_library(palex_objects ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_INLINE_FILES})
target_include_directories(palex_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(palex palex.cpp)
target_include_directories(palex PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(palex palex_objects)
