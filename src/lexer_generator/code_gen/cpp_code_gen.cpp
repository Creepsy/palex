#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <map>
#include <string_view>

#include "templates/template_completion.h"

#include "util/utf8.h"
#include "util/stream_format.h"
#include "util/palex_except.h"

#include "cpp_lexer_source.h"
#include "cpp_lexer_header.h"
#include "cpp_utf8_source.h"
#include "cpp_utf8_header.h"
#include "cpp_token_source.h"
#include "cpp_token_header.h"

constexpr size_t RESERVED_TOKEN_COUNT = 2;
const std::string RESTORE_FALLBACK_FUNC_COMPLETION = 
R"(void %UNIT_NAME%Lexer::try_restore_fallback() {
    if (!this->fallback.has_value()) {
        return;
    }
    this->curr_token = this->fallback.value();
    this->position = this->current_token().identifier.end();
    this->file_position = this->current_token().end;
})";
const std::string STATE_COMPLETION =
R"(case %STATE_ID%:
%STATE_CONTENT%
)";
const std::string STATE_TRANSITION_COMPLETION = 
R"(switch (current_codepoint) {
%STATE_TRANSITIONS%
}
)";

// helper functions
void complete_type_enum(const code_gen::TokenInfos& tokens, std::ostream& output);
void complete_token_type_strings(const code_gen::TokenInfos& tokens, std::ostream& output);
void complete_restore_token_fallback_function(const std::string& unit_name, const input::PalexConfig& config, std::ostream& output);
void complete_state(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const input::PalexConfig& config, 
    std::ostream& output
);
void complete_error_state(const std::string& unit_name, const input::PalexConfig& config, std::ostream& output);
void complete_state_transition_table(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
);
void complete_state_transition(
    const lexer_generator::LexerAutomaton_t::ConnectionID_t connection,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
);
void complete_state_default_transition(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
);
void complete_state_content(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    const input::PalexConfig& config,
    std::ostream& output
);

void complete_type_enum(const code_gen::TokenInfos& tokens, std::ostream& output) {
    output << sfmt::Indentation{3}; 
    output << "UNDEFINED,\n"
           << "END_OF_FILE";
    for (const std::string& token : tokens.tokens) {
        output << ",\n" << token;
    }
    for (const std::string& ignored_token : tokens.ignored_tokens) {
        output << ",\n" << ignored_token;
    }
    output << sfmt::Indentation{-3};
}

void complete_token_type_strings(const code_gen::TokenInfos& tokens, std::ostream& output) {
    output << sfmt::Indentation{1}; 
    output << "\"UNDEFINED\",\n"
           << "\"END_OF_FILE\"";
    for (const std::string& token : tokens.tokens) {
        output << ",\n\"" << token << '"';
    }
    for (const std::string& ignored_token : tokens.ignored_tokens) {
        output << ",\n\"" << ignored_token << '"';
    }
    output << sfmt::Indentation{-1}; 
}

void complete_restore_token_fallback_function(const std::string& unit_name, const input::PalexConfig& config, std::ostream& output) {
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
        {"UNIT_NAME", templates::constant_completer(unit_name)}
    };
    templates::write_template_to_stream(RESTORE_FALLBACK_FUNC_COMPLETION.c_str(), output, completers);
}

void complete_state(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const input::PalexConfig& config, 
    std::ostream& output
) {
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"STATE_ID", templates::constant_completer(std::to_string(state_id))},
        {
            "STATE_CONTENT",
            [&](std::ostream& output) {
                output << sfmt::Indentation{1};
                complete_state_content(unit_name, state_id, lexer_dfa, config, output);
                output << sfmt::Indentation{-1};
            }
        }
    };
    output << sfmt::Indentation{3};
    templates::write_template_to_stream(STATE_COMPLETION.c_str(), output, completers);
    output << sfmt::Indentation{-3};
}

void complete_error_state(const std::string& unit_name, const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{3};
    output << "case ERROR_STATE:\n";
    output << sfmt::Indentation{1};
    output << "this->curr_token = create_token(" << unit_name << "Token::TokenType::UNDEFINED);\n";
    if (config.lexer_fallback) {
        output << "this->try_restore_fallback();\n";
    }
    output << "return this->current_token().type;";
    output << sfmt::Indentation{-1};
    output << sfmt::Indentation{-3};
}

void complete_state_transition_table(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
) {
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {
            "STATE_TRANSITIONS", 
            [&](std::ostream& output) {
                output << sfmt::Indentation{1};
                for (const lexer_generator::LexerAutomaton_t::ConnectionID_t connection : lexer_dfa.get_outgoing_connection_ids(state_id)) {
                    complete_state_transition(connection, lexer_dfa, output);
                }
                complete_state_default_transition(unit_name, state_id, lexer_dfa, output);
                output << sfmt::Indentation{-1};
            }
        }
    };    
    templates::write_template_to_stream(STATE_TRANSITION_COMPLETION.c_str(), output, completers);
}

void complete_state_transition(
    const lexer_generator::LexerAutomaton_t::ConnectionID_t connection,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
) {
    assert(lexer_dfa.get_connection(connection).value.has_value() && "BUG: Found epsilon connection in DFA!");
    for (const regex::CharRange& range : lexer_dfa.get_connection(connection).value.value().get_ranges()) {
        output << "case " << (size_t)range.start;
        if (!range.is_single_char()) {
            output << " ... " << (size_t)range.end;
        }
        output << ":\n";
    }
    output << sfmt::Indentation{1};
    output << "state = " << lexer_dfa.get_connection(connection).target << ";\n";
    output << "break;\n";
    output << sfmt::Indentation{-1};
}

void complete_state_default_transition(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
) {
    output << "default:\n";
    output << sfmt::Indentation{1};
    if (lexer_dfa.get_state(state_id).empty()) {
        output << "state = ERROR_STATE;\n"
                << "break;";
    } else {
        output << "this->curr_token = create_token(" << unit_name << "Token::TokenType::" << lexer_dfa.get_state(state_id)<< ");\n"
               << "return this->current_token().type;";
    }
    output << sfmt::Indentation{-1};
}

void complete_state_content(
    const std::string& unit_name,
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    const input::PalexConfig& config,
    std::ostream& output
) {

    if (!lexer_dfa.has_outgoing_connections(state_id)) {
        assert(!lexer_dfa.get_state(state_id).empty() && "BUG: Found DFA with empty leaf node!");
        output << "this->curr_token = create_token(" << unit_name << "Token::TokenType::" << lexer_dfa.get_state(state_id) << ");\n"
               << "return this->current_token().type;";
        return;
    }
    if (config.lexer_fallback && !lexer_dfa.get_state(state_id).empty()) {
        output << "this->fallback = create_token(" << unit_name << "Token::TokenType::" << lexer_dfa.get_state(state_id) << ");\n";
    }
    complete_state_transition_table(unit_name, state_id, lexer_dfa, output);
    output << "break;";
}

bool code_gen::cpp::generate_lexer_files(
    const std::vector<lexer_generator::TokenDefinition>& token_definitions,
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const std::string& unit_name, 
    const input::PalexConfig& config
) {
    const TokenInfos& tokens = code_gen::conv_rules_to_generation_info(token_definitions);
    try {
        generate_lexer_header(unit_name, config, tokens);
        generate_lexer_source(tokens, lexer_dfa, unit_name, config);
        generate_token_header(unit_name, config, tokens);
        generate_token_source(unit_name, config, tokens);
        if (config.generate_util) {
            generate_utf8_lib(config);
        }
    } catch(const std::exception& e) {
        std::cerr << "Failed to generate (some) lexer files: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void code_gen::cpp::generate_lexer_header(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.output_path + "/" + unit_name + "Lexer.h";
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"FALLBACK_INCLUDES", templates::conditional_completer(config.lexer_fallback, "\n#include <optional>")},
        {"MODULE_NAME", templates::constant_completer(config.module_name)},
        {"UNIT_NAME", templates::constant_completer(unit_name)},
        {"FALLBACK_FUNCTION", templates::conditional_completer(config.lexer_fallback, "void try_restore_fallback();\n")},
        {"FALLBACK_CACHE", templates::conditional_completer(config.lexer_fallback, "std::optional<" + unit_name + "Token> fallback;")}
    };
    std::cout << "Generating file " << header_file_path << "..." << std::endl;
    templates::write_template_to_file(cpp_lexer_header, header_file_path, completers);
}

void code_gen::cpp::generate_lexer_source(
    const TokenInfos& tokens, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    const std::string& unit_name, 
    const input::PalexConfig& config
) {
    using namespace std::placeholders;

    const std::string source_file_path = config.output_path + "/" + unit_name + "Lexer.cpp";
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"UNIT_NAME", templates::constant_completer(unit_name)},
        {"MODULE_NAME", templates::constant_completer(config.module_name)},
        {"FALLBACK_CLEAR", templates::conditional_completer(config.lexer_fallback, "this->fallback = std::nullopt;")},
        {
            "STATES", 
            [&](std::ostream& output) {
                for (const auto& [state_id, _] : lexer_dfa.get_states()) {
                    complete_state(unit_name, state_id, lexer_dfa, config, output);
                }
            }
        },
        {"ERROR_STATE", std::bind(complete_error_state, unit_name, config, _1)},
        {
            "FALLBACK_FUNCTION", 
            config.lexer_fallback ? std::bind(complete_restore_token_fallback_function, unit_name, config, _1)
                                  : templates::EMPTY_COMPLETER
        }
    };
    std::cout << "Generating file " << source_file_path << "..." << std::endl;
    templates::write_template_to_file(cpp_lexer_source, source_file_path, completers);
}

void code_gen::cpp::generate_token_header(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.output_path + "/" + unit_name + "Token.h";
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"UNIT_NAME", templates::constant_completer(unit_name)},
        {"MODULE_NAME", templates::constant_completer(config.module_name)},
        {"TOKEN_TYPE_ENUM", std::bind(complete_type_enum, tokens, _1)}
    };
    templates::write_template_to_file(cpp_token_header, header_file_path, completers);
}

void code_gen::cpp::generate_token_source(const std::string& unit_name, const input::PalexConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;
   
    const std::string source_file_path = config.output_path + "/" + unit_name + "Token.cpp";
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"UNIT_NAME", templates::constant_completer(unit_name)},
        {"MODULE_NAME", templates::constant_completer(config.module_name)},
        {"LAST_NORMAL_TOKEN", templates::constant_completer(tokens.tokens.empty() ? "END_OF_FILE" : tokens.tokens.back())},
        {"TOKEN_COUNT", templates::constant_completer(std::to_string(tokens.tokens.size() + tokens.ignored_tokens.size() + RESERVED_TOKEN_COUNT))},
        {"TOKEN_STRINGS", std::bind(complete_token_type_strings, tokens, _1)},
    };
    templates::write_template_to_file(cpp_token_source, source_file_path, completers);
}

void code_gen::cpp::generate_utf8_lib(const input::PalexConfig& config) { // TODO: move to extra generator for lib files?
    const std::string utf8_path = config.util_output_path + "/utf8";
    std::cout << "Generating file " << utf8_path << ".cpp..." << std::endl;
    templates::write_template_to_file(cpp_utf8_source, utf8_path + ".cpp", {});
    std::cout << "Generating file " << utf8_path << ".h..." << std::endl;
    templates::write_template_to_file(cpp_utf8_header, utf8_path + ".h", {});
}