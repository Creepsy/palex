set(PROJECT_SOURCES
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
    
    parser_generator/validation.h
    parser_generator/production_definition.h

    parser_generator/shift_reduce_parsers/parser_state.h
    parser_generator/shift_reduce_parsers/parser_table_generation.h
    parser_generator/shift_reduce_parsers/parser_state_comparators.h
    parser_generator/shift_reduce_parsers/state_lookahead.h

    parser_generator/shift_reduce_parsers/code_gen/parser_generation.h
    parser_generator/shift_reduce_parsers/code_gen/cpp_code_gen.h

    input/cmd_arguments.h
)

set(PROJECT_INLINE_FILES
    util/Automaton.ipp

    regex/RegexParser.ipp
)

set(AUTO_GENERATED_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRuleLexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRuleLexer.h
)

include(templates/all_template_headers.cmake)

add_library(palex_objects ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_INLINE_FILES})
target_include_directories(palex_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_custom_command(
    OUTPUT ${AUTO_GENERATED_FILES}
    COMMAND ${CMAKE_BINARY_DIR}/lexergen ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRule.palex -lang c++ -output-path ${CMAKE_CURRENT_SOURCE_DIR}/input 
                -util-path util -module-name input --lexer --fallback
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/input/PalexRule.palex 
)
add_library(auto_generated_objects ${AUTO_GENERATED_FILES})
target_include_directories(auto_generated_objects PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(auto_generated_objects palex_objects)

add_executable(lexergen lexergen.cpp)
target_include_directories(lexergen PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(lexergen palex_objects auto_generated_objects)

add_executable(palex palex.cpp)
target_include_directories(palex PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(palex palex_objects auto_generated_objects)

install(TARGETS palex DESTINATION bin)