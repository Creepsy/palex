set(PROJECT_SOURCES
    util/utf8.cpp
    util/palex_except.cpp
    util/config_getters.cpp
    util/config_loader.cpp

    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/lexer_automaton.cpp

    lexer_generator/lang/LexerRuleLexer.cpp
    lexer_generator/lang/LexerRuleParser.cpp
    lexer_generator/lang/validation.cpp

    lexer_generator/code_gen/cpp_code_gen.cpp
    lexer_generator/code_gen/lexer_generation.cpp
    lexer_generator/code_gen/code_gen_data.cpp

    templates/template_completion.cpp

    parser_generator/lang/ParserProductionParser.cpp
    parser_generator/lang/validation.cpp

    parser_generator/shift_reduce_parsers/parser_tables.cpp
    parser_generator/shift_reduce_parsers/parser_table_generation.cpp
)

set(PROJECT_HEADERS
    util/Automaton.h
    util/utf8.h
    util/palex_except.h
    util/config_getters.h
    util/config_loader.h
    util/Visitor.h

    regex/character_classes.h
    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/lexer_automaton.h

    lexer_generator/lang/LexerRuleLexer.h
    lexer_generator/lang/LexerRuleParser.h
    lexer_generator/lang/validation.h

    lexer_generator/code_gen/cpp_code_gen.h
    lexer_generator/code_gen/lexer_generation.h
    lexer_generator/code_gen/code_gen_data.h

    templates/template_completion.h

    ${CMAKE_BINARY_DIR}/cpp_lexer_source.h
    ${CMAKE_BINARY_DIR}/cpp_lexer_header.h
    ${CMAKE_BINARY_DIR}/cpp_utf8_header.h
    ${CMAKE_BINARY_DIR}/cpp_utf8_source.h
    
    ${CMAKE_SOURCE_DIR}/vendor/json.h

    parser_generator/lang/ParserProductionParser.h
    parser_generator/lang/validation.h

    parser_generator/shift_reduce_parsers/parser_tables.h
    parser_generator/shift_reduce_parsers/parser_table_generation.h
)

set(PROJECT_INLINE_FILES
    util/Automaton.ipp

    regex/RegexParser.ipp
)

set(AUTO_GENERATED_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/lang/ParserProductionLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/lang/ParserProductionLexer.h
)

include(templates/all_template_headers.cmake)

add_library(palex_objects ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_INLINE_FILES})
target_include_directories(palex_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(lexergen lexergen.cpp)
target_include_directories(lexergen PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(lexergen palex_objects)

# use add_custom_target instead of add_custom_command as it lead to build order conflicts. Is there any better solution?
add_custom_target(
    autogen_objects
    COMMAND ${CMAKE_BINARY_DIR}/lexergen ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/lang
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/lang/palex.cfg ${CMAKE_CURRENT_SOURCE_DIR}/parser_generator/lang/ParserProductionLexer.lrules
)
add_library(auto_generated_objects ${AUTO_GENERATED_FILES})
target_link_libraries(auto_generated_objects PRIVATE palex_objects)
target_include_directories(auto_generated_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_executable(palex palex.cpp)
target_include_directories(palex PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(palex palex_objects auto_generated_objects)
add_dependencies(palex autogen_objects)