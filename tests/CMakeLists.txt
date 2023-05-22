function(generate_test_executable test_name test_src)
    add_executable(${test_name} ${test_src})
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${test_name} palex_objects auto_generated_objects)
    add_test(NAME ${test_name} COMMAND ${test_name})  
endfunction()

generate_test_executable(PalexRuleLexer input/palex_rule_lexer_test.cpp)
generate_test_executable(PalexRuleParser input/palex_rule_parser_test.cpp)
generate_test_executable(PalexRuleParserErrors input/palex_rule_parser_errors_test.cpp)

generate_test_executable(UTF8Input util/utf8_input_test.cpp)
generate_test_executable(UTF8Output util/utf8_output_test.cpp)
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