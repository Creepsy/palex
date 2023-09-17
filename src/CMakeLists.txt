set(PROJECT_SOURCES
    command_line_interface.cpp

    bootstrap/BootstrapLexer.cpp
    bootstrap/TokenInfo.cpp

    util/utf8.cpp
    util/palex_except.cpp
    util/stream_format.cpp

    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/lexer_automaton.cpp
    lexer_generator/validation.cpp

    lexer_generator/code_gen/cpp_code_gen.cpp
    lexer_generator/code_gen/lexer_generation.cpp
    lexer_generator/code_gen/code_gen_data.cpp

    templates/template_completion.cpp

    parser_generator/validation.cpp
    parser_generator/production_definition.cpp

    parser_generator/shift_reduce_parsers/parser_state.cpp
    parser_generator/shift_reduce_parsers/parser_table_generation.cpp
    parser_generator/shift_reduce_parsers/parser_state_comparators.cpp
    parser_generator/shift_reduce_parsers/state_lookahead.cpp

    parser_generator/shift_reduce_parsers/code_gen/parser_generation.cpp
    parser_generator/shift_reduce_parsers/code_gen/cpp_code_gen.cpp

    input/PalexRuleParser.cpp
    input/cmd_arguments.cpp
)

set(PROJECT_HEADERS
    command_line_interface.h

    bootstrap/BootstrapLexer.h
    bootstrap/TokenInfo.h

    util/Automaton.h
    util/utf8.h
    util/palex_except.h
    util/Visitor.h
    util/stream_format.h

    regex/character_classes.h
    regex/regex_ast.cpp
    regex/RegexParser.cpp

    lexer_generator/lexer_automaton.h
    lexer_generator/validation.h
    lexer_generator/token_definition.h

    lexer_generator/code_gen/cpp_code_gen.h
    lexer_generator/code_gen/lexer_generation.h
    lexer_generator/code_gen/code_gen_data.h

    templates/template_completion.h

    ${CMAKE_BINARY_DIR}/cpp_lexer_source.h
    ${CMAKE_BINARY_DIR}/cpp_lexer_header.h
    ${CMAKE_BINARY_DIR}/cpp_utf8_header.h
    ${CMAKE_BINARY_DIR}/cpp_utf8_source.h
    ${CMAKE_BINARY_DIR}/cpp_parser_header.h
    ${CMAKE_BINARY_DIR}/cpp_parser_source.h
    ${CMAKE_BINARY_DIR}/cpp_types_header.h
    ${CMAKE_BINARY_DIR}/cpp_types_source.h
    ${CMAKE_BINARY_DIR}/cpp_token_header.h
    ${CMAKE_BINARY_DIR}/cpp_token_source.h
    
    parser_generator/validation.h
    parser_generator/production_definition.h

    parser_generator/shift_reduce_parsers/parser_state.h
    parser_generator/shift_reduce_parsers/parser_table_generation.h
    parser_generator/shift_reduce_parsers/parser_state_comparators.h
    parser_generator/shift_reduce_parsers/state_lookahead.h

    parser_generator/shift_reduce_parsers/code_gen/parser_generation.h
    parser_generator/shift_reduce_parsers/code_gen/cpp_code_gen.h

    input/PalexRuleParser.h
    input/cmd_arguments.h
)

set(PROJECT_INLINE_FILES
    util/Automaton.ipp

    regex/RegexParser.ipp
)

set(LEXER_AUTOGEN_FILES
    ${CMAKE_BINARY_DIR}/PalexRuleLexer.cpp
    ${CMAKE_BINARY_DIR}/PalexRuleLexer.h
    ${CMAKE_BINARY_DIR}/PalexRuleToken.cpp
    ${CMAKE_BINARY_DIR}/PalexRuleToken.h
)
set(LEXER_ADAPTER_FILES
    input/PalexLexerAdapter.cpp
    input/PalexLexerAdapter.h
)

include(templates/all_template_headers.cmake)

add_library(palex_objects ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_INLINE_FILES})
target_include_directories(palex_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_custom_command(
    OUTPUT ${LEXER_AUTOGEN_FILES}
    COMMAND ${CMAKE_BINARY_DIR}/lexergen ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRule.palex -lang c++ -output-path ${CMAKE_BINARY_DIR}
                -util-path ${CMAKE_CURRENT_SOURCE_DIR}/util -module-name input --lexer --fallback
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRule.palex lexergen
)
add_library(lexer_autogen ${LEXER_AUTOGEN_FILES} ${LEXER_ADAPTER_FILES})
target_include_directories(lexer_autogen PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(lexer_autogen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/util) # for utf8 library linking of generated lexer
target_link_libraries(lexer_autogen palex_objects)

add_executable(lexergen lexergen.cpp)
target_include_directories(lexergen PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(lexergen palex_objects)

add_executable(palex palex.cpp)
target_include_directories(palex PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(palex palex_objects lexer_autogen)

install(TARGETS palex DESTINATION bin)