#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <filesystem>
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

constexpr size_t RESERVED_TOKEN_COUNT = 2;
const std::string FALLBACK_TYPES_COMPLETION =
R"(struct Fallback {
    size_t token_length;
    CharacterPosition next_token_position;
    Token::TokenType type;
};

Token try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start);)";
const std::string RESTORE_FALLBACK_FUNC_COMPLETION = 
R"(%LEXER_NAMESPACE%::Token %LEXER_NAMESPACE%::%LEXER_NAME%::try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start) {
    if (!this->fallback.has_value()) return Token{Token::TokenType::UNDEFINED, token_identifier, token_start};
    while (token_identifier.size() > this->fallback.value().token_length) {
        this->cache.push(token_identifier.back());
        token_identifier.pop_back();
    }
    this->curr_position = this->fallback.value().next_token_position;
    return Token{this->fallback.value().type, token_identifier, token_start};
})";
const std::string STATE_COMPLETION =
R"(case %STATE_ID%:
%STATE_CONTENT%
)";
const std::string STATE_TRANSITION_COMPLETION = 
R"(switch (curr) {
%STATE_TRANSITIONS%
}
)";

// helper functions
void complete_type_enum(const code_gen::TokenInfos& tokens, std::ostream& output);
void complete_token_type_strings(const code_gen::TokenInfos& tokens, std::ostream& output);
void complete_restore_token_fallback_function(const std::string& lexer_name, const input::PalexConfig& config, std::ostream& output);
void complete_state_machine(const lexer_generator::LexerAutomaton_t& lexer_dfa, const input::PalexConfig& config, std::ostream& output);
void complete_state(
    const lexer_generator::LexerAutomaton_t::StateID_t state_id, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const input::PalexConfig& config, 
    std::ostream& output
);
void complete_error_state(const input::PalexConfig& config, std::ostream& output);
void complete_state_transition_table(
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
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    std::ostream& output
);
void complete_state_content(
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

void complete_restore_token_fallback_function(const std::string& lexer_name, const input::PalexConfig& config, std::ostream& output) {
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"LEXER_NAMESPACE", templates::constant_completer(config.module_name)},
        {"LEXER_NAME", templates::constant_completer(lexer_name)}
    };
    templates::write_template_to_stream(RESTORE_FALLBACK_FUNC_COMPLETION.c_str(), output, completers);
}

void complete_state(
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
                complete_state_content(state_id, lexer_dfa, config, output);
                output << sfmt::Indentation{-1};
            }
        }
    };
    output << sfmt::Indentation{3};
    templates::write_template_to_stream(STATE_COMPLETION.c_str(), output, completers);
    output << sfmt::Indentation{-3};
}

void complete_error_state(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{3};
    output << "case ERROR_STATE:\n";
    output << sfmt::Indentation{1};
    if (config.lexer_fallback) {
        output << "return this->try_restore_fallback(identifier, token_start);";
    } else {
        output << "return Token{Token::TokenType::UNDEFINED, identifier, token_start};";   
    }
    output << sfmt::Indentation{-1};
    output << sfmt::Indentation{-3};
}

void complete_state_transition_table(
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
                complete_state_default_transition(state_id, lexer_dfa, output);
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
        output << "return Token{Token::TokenType::" << lexer_dfa.get_state(state_id) << ", identifier, token_start};";
    }
    output << sfmt::Indentation{-1};
}

void complete_state_content(
    const lexer_generator::LexerAutomaton_t::StateID_t state_id,
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    const input::PalexConfig& config,
    std::ostream& output
) {
    if (!lexer_dfa.has_outgoing_connections(state_id)) {
        assert(!lexer_dfa.get_state(state_id).empty() && "BUG: Found DFA with empty leaf node!");
        output << "return Token{Token::TokenType::" << lexer_dfa.get_state(state_id) << ", identifier, token_start};";
        return;
    }
    if (config.lexer_fallback && !lexer_dfa.get_state(state_id).empty()) {
        output << "this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::"
               << lexer_dfa.get_state(state_id) << "};\n";
    }
    complete_state_transition_table(state_id, lexer_dfa, output);
    output << "break;";
}

bool code_gen::cpp::generate_lexer_files(
    const std::vector<lexer_generator::TokenDefinition>& token_definitions,
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const std::string& lexer_name, 
    const input::PalexConfig& config
) {
    const TokenInfos& tokens = code_gen::conv_rules_to_generation_info(token_definitions);
    try {
        generate_lexer_header(lexer_name, config, tokens);
        generate_lexer_source(tokens, lexer_dfa, lexer_name, config);
        if (config.generate_util) {
            generate_utf8_lib(config);
        }
    } catch(const std::exception& e) {
        std::cerr << "Failed to generate (some) lexer files: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void code_gen::cpp::generate_lexer_header(const std::string& lexer_name, const input::PalexConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.output_path + "/" + lexer_name + ".h";
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"FALLBACK_INCLUDES", templates::conditional_completer(config.lexer_fallback, "#include <optional>\n#include <utility>")},
        {"LEXER_NAMESPACE", templates::constant_completer(config.module_name)},
        {"TOKEN_TYPE_ENUM", std::bind(complete_type_enum, tokens, _1)},
        {"LEXER_NAME", templates::constant_completer(lexer_name)},
        {
            "FALLBACK_TYPES", 
            [&](std::ostream& output) {
                if (!config.lexer_fallback) {
                    return;
                }
                output << sfmt::Indentation{3};
                output << FALLBACK_TYPES_COMPLETION;
                output << sfmt::Indentation{-3};
            }
        },
        {"FALLBACK_CACHE", templates::conditional_completer(config.lexer_fallback, "std::optional<Fallback> fallback;")}
    };
    std::cout << "Generating file " << header_file_path << "..." << std::endl;
    templates::write_template_to_file(cpp_lexer_header, header_file_path, completers);
}

void code_gen::cpp::generate_lexer_source(
    const TokenInfos& tokens, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa,
    const std::string& lexer_name, 
    const input::PalexConfig& config
) {
    using namespace std::placeholders;

    const std::string source_file_path = config.output_path + "/" + lexer_name + ".cpp";
    const std::string utf8_lib_path = std::filesystem::relative(
                std::filesystem::absolute(config.util_output_path),
                std::filesystem::absolute(config.output_path)
            ).string();
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
        {"LEXER_NAME", templates::constant_completer(lexer_name)},
        {"UTF8_LIB_PATH", templates::constant_completer(utf8_lib_path)},
        {"LEXER_NAMESPACE", templates::constant_completer(config.module_name)},
        {"TOKEN_TYPE_COUNT", templates::constant_completer(std::to_string(tokens.tokens.size() + tokens.ignored_tokens.size() + RESERVED_TOKEN_COUNT))},
        {"TOKEN_TYPE_STRINGS", std::bind(complete_token_type_strings, tokens, _1)},
        {"LAST_NORMAL_TOKEN", templates::constant_completer(tokens.tokens.empty() ? "END_OF_FILE" : tokens.tokens.back())},
        {"PRE_STATE_MACHINE_FALLBACK", templates::conditional_completer(config.lexer_fallback, "this->fallback = std::nullopt;")},
        {
            "STATES", 
            [&](std::ostream& output) {
                for (const auto& [state_id, _] : lexer_dfa.get_states()) {
                    complete_state(state_id, lexer_dfa, config, output);
                }
            }
        },
        {"ERROR_STATE", std::bind(complete_error_state, config, _1)},
        {
            "RESTORE_FALLBACK_FUNC", 
            config.lexer_fallback ? std::bind(complete_restore_token_fallback_function, lexer_name, config, _1)
                                  : templates::EMPTY_COMPLETER
        }
    };
    std::cout << "Generating file " << source_file_path << "..." << std::endl;
    templates::write_template_to_file(cpp_lexer_source, source_file_path, completers);
}

void code_gen::cpp::generate_utf8_lib(const input::PalexConfig& config) {
    const std::string utf8_path = config.util_output_path + "/utf8";
    std::cout << "Generating file " << utf8_path << ".cpp..." << std::endl;
    templates::write_template_to_file(cpp_utf8_source, utf8_path + ".cpp", {});
    std::cout << "Generating file " << utf8_path << ".h..." << std::endl;
    templates::write_template_to_file(cpp_utf8_header, utf8_path + ".h", {});
}