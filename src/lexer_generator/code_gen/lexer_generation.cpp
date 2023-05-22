#include "lexer_generation.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <cassert>
#include <cstddef>

#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "lexer_generator/token_definition.h"
#include "lexer_generator/validation.h"

#include "util/palex_except.h"

#include "cpp_code_gen.h"

const std::vector<code_gen::LexerCodeGenerator_t> code_gen::LANGUAGE_CODE_GENERATORS = {
    code_gen::EMPTY_LEXER_GENERATOR,
    code_gen::cpp::generate_cpp_lexer_files
    // TODO: add cpp lexer generator
    // {"c++", code_gen::cpp::generate_cpp_lexer_files}, // TODO: first empty
    // {"C++", code_gen::cpp::generate_cpp_lexer_files},
    // {"cpp", code_gen::cpp::generate_cpp_lexer_files},
    // {"CPP", code_gen::cpp::generate_cpp_lexer_files}
};

// helper functions
// std::vector<lexer_generator::TokenDefinition> parse_lexer_rules(std::istream& rule_input);
lexer_generator::LexerAutomaton_t generate_dfa_from_rules(const std::vector<lexer_generator::TokenDefinition>& lexer_rules);

// std::vector<lexer_generator::TokenDefinition> parse_lexer_rules(std::istream& rule_input) {
//     input::PalexRuleLexer lexer(rule_input);
//     input::PalexRuleParser parser(lexer);
    
//     return parser.parse_all_token_definitions(); // TODO
// }

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

bool code_gen::generate_lexer(const std::string& lexer_name, const std::vector<lexer_generator::TokenDefinition>& token_definitions, const input::PalexConfig& config) {
    if (token_definitions.empty()) {
        std::cerr << "Skipped generation of lexer as it's rule file is empty!" << std::endl;
        return false;
    }
    lexer_generator::validate_rules(token_definitions);
    lexer_generator::LexerAutomaton_t lexer_dfa = generate_dfa_from_rules(token_definitions);
    assert(LANGUAGE_CODE_GENERATORS.size() > (size_t)config.language && "BUG: Supplied language has no generator associated with it!");
    return LANGUAGE_CODE_GENERATORS[(size_t)config.language](token_definitions, lexer_dfa, lexer_name, config);
}
