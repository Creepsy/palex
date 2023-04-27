#include "lexer_generation.h"

#include <fstream>
#include <iostream>
#include <filesystem>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "lexer_generator/token_definition.h"
#include "lexer_generator/validation.h"

#include "util/palex_except.h"

#include "cpp_code_gen.h"

const std::map<std::string, code_gen::LexerCodeGenerator_t> code_gen::LANGUAGE_CODE_GENERATORS = {
    {"c++", code_gen::cpp::generate_cpp_lexer_files},
    {"C++", code_gen::cpp::generate_cpp_lexer_files},
    {"cpp", code_gen::cpp::generate_cpp_lexer_files},
    {"CPP", code_gen::cpp::generate_cpp_lexer_files}
};

// helper functions
std::vector<lexer_generator::TokenDefinition> parse_lexer_rules(std::istream& rule_input);
lexer_generator::LexerAutomaton_t generate_dfa_from_rules(const std::vector<lexer_generator::TokenDefinition>& lexer_rules);

std::vector<lexer_generator::TokenDefinition> parse_lexer_rules(std::istream& rule_input) {
    input::PalexRuleLexer lexer(rule_input);
    input::PalexRuleParser parser(lexer);
    
    return parser.parse_all_token_definitions(); // TODO
}

lexer_generator::LexerAutomaton_t generate_dfa_from_rules(const std::vector<lexer_generator::TokenDefinition>& lexer_rules) {
    using namespace std::placeholders;
    
    lexer_generator::LexerAutomaton_t lexer_nfa{};
    const lexer_generator::LexerAutomaton_t::StateID_t root_state = lexer_nfa.add_state("");


    for (const lexer_generator::TokenDefinition& rule : lexer_rules) {
        lexer_generator::insert_rule_in_nfa(lexer_nfa, root_state, rule);
    }

    std::map<std::string, size_t> token_priorities = lexer_generator::get_token_priorities(lexer_rules);
    auto merge_states = std::bind(lexer_generator::merge_states_by_priority, token_priorities, _1);

    return lexer_nfa.convert_to_dfa<std::string>(
        root_state,
        merge_states,
        lexer_generator::resolve_connection_collisions
    );
}



bool code_gen::generate_lexer(const std::string& lexer_name, const nlohmann::json& json_config, const std::string& target_language) {
    const std::string lexer_rule_path = std::filesystem::current_path().string() + "/" + lexer_name + ".lrules"; 

    std::ifstream lexer_rule_file{lexer_rule_path};

    if (!lexer_rule_file.is_open()) {
        std::cerr << "Unable to open lexer rule file '" << lexer_rule_path << "'!" << std::endl; 
        
        return false;
    }

    std::vector<lexer_generator::TokenDefinition> lexer_rules;

    try {
        lexer_rules = parse_lexer_rules(lexer_rule_file);
    } catch(const palex_except::ParserError& e) {
        lexer_rule_file.close();
        std::cerr << "Error while parsing rules of '" << lexer_name << "': " << e.what() << std::endl;

        return false;
    }
    lexer_rule_file.close();

    if (lexer_rules.empty()) {
        std::cerr << "Skipped generation of lexer '" << lexer_name + "' as it's rule file is empty!" << std::endl;

        return false;
    }

    try {
        lexer_generator::validate_rules(lexer_rules);
    } catch(const palex_except::ValidationError& e) {
        std::cerr << "The lexer '" << lexer_name << "' has conflicting rules: " << e.what() << std::endl;

        return false;
    }

    lexer_generator::LexerAutomaton_t lexer_dfa;

    try {
        lexer_dfa = generate_dfa_from_rules(lexer_rules);
    } catch(const palex_except::ValidationError& e) {
        std::cerr << "The lexer '" << lexer_name << "' has conflicting rules: " << e.what() << std::endl;

        return false;
    }

    if (LANGUAGE_CODE_GENERATORS.find(target_language) == LANGUAGE_CODE_GENERATORS.end()) {
        std::cerr << "Lexer code generation for the language '" << target_language << "' is not supported!" << std::endl;

        return false;
    }

    return LANGUAGE_CODE_GENERATORS.at(target_language)(lexer_rules, lexer_dfa, lexer_name, json_config);
}
