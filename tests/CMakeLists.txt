include(generated_lexer/create_lexer_test.cmake)

function(generate_test_executable test_name test_src)
    add_executable(${test_name} ${test_src})
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${test_name} palex_objects lexer_autogen)
    add_test(NAME ${test_name} COMMAND ${test_name})  
endfunction()

generate_test_executable(PalexRuleLexer input/palex_rule_lexer_test.cpp)
generate_test_executable(PalexRuleParser input/palex_rule_parser_test.cpp)
generate_test_executable(PalexRuleParserErrors input/palex_rule_parser_errors_test.cpp)
generate_test_executable(CmdArguments input/cmd_arguments_test.cpp)
generate_test_executable(CmdArgumentsErrors input/cmd_arguments_errors_test.cpp)

generate_test_executable(UTF8Input util/utf8_input_test.cpp)
generate_test_executable(AutomatonStates util/automaton_states_test.cpp)
generate_test_executable(AutomatonConnections util/automaton_connections_test.cpp)
generate_test_executable(AutomatonDFAConversion util/automaton_dfa_conversion_test.cpp)

generate_test_executable(LexerRuleValidation lexer_generator/rule_validation_test.cpp)

generate_test_executable(RegexCharRange regex/char_range_test.cpp)
generate_test_executable(RegexCharRangeSet regex/char_range_set_test.cpp)
generate_test_executable(RegexErrors regex/regex_errors_test.cpp)
generate_test_executable(RegexCharSet regex/char_set_test.cpp)
generate_test_executable(RegexBranch regex/branch_test.cpp)
generate_test_executable(RegexSequence regex/sequence_test.cpp)
generate_test_executable(RegexQuantifier regex/quantifier_test.cpp)
generate_test_executable(RegexPriority regex/regex_priority_test.cpp)

generate_test_executable(TemplateCompletion templates/template_completion_test.cpp)
generate_test_executable(TemplateErrors templates/template_errors_test.cpp)
generate_test_executable(TemplateEscapeCharacters templates/escape_characters_test.cpp)

generate_test_executable(ParserProductionValidation parser_generator/production_validation_test.cpp)
generate_test_executable(ActionConflict parser_generator/action_conflict_test.cpp)
generate_test_executable(FirstSet parser_generator/first_set_test.cpp)
generate_test_executable(FirstSetError parser_generator/first_set_error_test.cpp)
generate_test_executable(FollowTerminals parser_generator/follow_terminals_test.cpp)
generate_test_executable(GrammarConflictLALR parser_generator/grammar_conflict_test.cpp)
generate_test_executable(ParserTableTest parser_generator/parser_table_test.cpp)
generate_test_executable(ParserTableErrorTest parser_generator/parser_table_error_test.cpp)

create_lexer_test(
    LexerIntIdentTest
    generated_lexer/int_ident_test.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/IntIdentTest.palex 
    "--fallback"
)
create_lexer_output_test(
    LexerIntIdentTest
    generated_lexer/int_ident_test.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/IntIdentTest.palex 
    "--fallback"
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/int_ident_test.out
)
create_lexer_output_test(
    IgnoredTokensTest
    generated_lexer/ignored_tokens_test.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/IgnoredTokensTest.palex 
    "--fallback"
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/ignored_tokens_test.out
)
create_lexer_output_test(
    FallbackTest
    generated_lexer/fallback_test.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/FallbackTest.palex 
    "--fallback"
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/fallback_test.out
)
create_lexer_output_test(
    NoFallbackTest
    generated_lexer/no_fallback_test.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/NoFallbackTest.palex 
    ""
    ${CMAKE_CURRENT_SOURCE_DIR}/generated_lexer/no_fallback_test.out
)