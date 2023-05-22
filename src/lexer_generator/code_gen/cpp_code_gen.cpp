#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <filesystem>

#include "templates/template_completion.h"

#include "util/utf8.h"
#include "util/palex_except.h"

#include "cpp_lexer_source.h"
#include "cpp_lexer_header.h"
#include "cpp_utf8_source.h"
#include "cpp_utf8_header.h"

// helper functions
bool complete_fallback_header_tags(std::ostream& output, const std::string_view tag);
bool complete_fallback_source_tags(std::ostream& output, const std::string_view tag, const std::string& lexer_name, const input::PalexConfig& config);
bool complete_generic_lexer_tags(std::ostream& output, const std::string_view tag, const std::string& lexer_name, const input::PalexConfig& config);

bool complete_fallback_header_tags(std::ostream& output, const std::string_view tag) {
    if (tag == "FALLBACK_CACHE") {
        output << "std::optional<Fallback> fallback;";
    } else if (tag == "FALLBACK_INCLUDES") {
        output << "#include <optional>\n"
               << "#include <utility>";
    } else if (tag == "FALLBACK_TYPES") {
        output << "struct Fallback {\n"
               << "                size_t token_length;\n"
               << "                CharacterPosition next_token_position;\n"
               << "                Token::TokenType type;\n"
               << "            };\n\n"
               << "            Token try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start);";
    } else {
        return false;
    }

    return true;
}

bool complete_fallback_source_tags(std::ostream& output, const std::string_view tag, const std::string& lexer_name, const input::PalexConfig& config) {
    if (tag == "RESTORE_FALLBACK_FUNC") {
        output << config.module_name << "::Token " << config.module_name << "::" << lexer_name
               << "::try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start) {\n"
               << "    if (!this->fallback.has_value()) return Token{Token::TokenType::UNDEFINED, token_identifier, token_start};\n\n"
               << "    while (token_identifier.size() > this->fallback.value().token_length) {\n"
               << "        this->cache.push(token_identifier.back());\n"
               << "        token_identifier.pop_back();\n"
               << "    }\n\n"
               << "    this->curr_position = this->fallback.value().next_token_position;\n\n"
               << "    return Token{this->fallback.value().type, token_identifier, token_start};\n"
               << "}";
    } else if (tag == "PRE_STATE_MACHINE_FALLBACK") {
        output << "this->fallback = std::nullopt;";
    } else {
        return false;
    }

    return true;
}

bool complete_generic_lexer_tags(std::ostream& output, const std::string_view tag, const std::string& lexer_name, const input::PalexConfig& config) {
    if (tag == "LEXER_NAMESPACE") {
        output << config.module_name;
    } else if (tag == "LEXER_NAME") {
        output << lexer_name;
    } else {
        return false;
    }

    return true;
}



bool code_gen::cpp::generate_cpp_lexer_files(
    const std::vector<lexer_generator::TokenDefinition>& lexer_rules, 
    const lexer_generator::LexerAutomaton_t& lexer_dfa, 
    const std::string& lexer_name, 
    const input::PalexConfig& config
) {
    try {
        code_gen::cpp::generate_lexer_files(lexer_dfa, lexer_name, config, code_gen::conv_rules_to_generation_info(lexer_rules));
    } catch(const std::exception& e) {
        std::cerr << "Failed to generate (some) lexer files: " << e.what() << std::endl;

        return false;
    }

    return true;
}

void code_gen::cpp::generate_lexer_files(
    const lexer_generator::LexerAutomaton_t& dfa, 
    const std::string& lexer_name, 
    const input::PalexConfig& config, 
    const TokenInfos& tokens
) {
    generate_lexer_header(lexer_name, config, tokens);
    generate_lexer_source(lexer_name, config, tokens, dfa);

    if (config.generate_util) {
        generate_utf8_lib(config);
    }
}

void code_gen::cpp::generate_lexer_header(const std::string& lexer_name, const input::PalexConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.output_path + "/" + lexer_name + ".h";
    std::cout << "Generating file " << header_file_path << "..." << std::endl;

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_header, _1, _2, lexer_name, config, tokens);
    templates::write_template_to_file(cpp_lexer_header, header_file_path, completer);
}

void code_gen::cpp::generate_lexer_source(
    const std::string& lexer_name, 
    const input::PalexConfig& config, 
    const TokenInfos& tokens, 
    const lexer_generator::LexerAutomaton_t& dfa
) {
    using namespace std::placeholders;

    const std::string source_file_path = config.output_path + "/" + lexer_name + ".cpp";
    std::cout << "Generating file " << source_file_path << "..." << std::endl;

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_source, _1, _2, lexer_name, config, tokens, dfa);
    templates::write_template_to_file(cpp_lexer_source, source_file_path, completer);
}

void code_gen::cpp::generate_utf8_lib(const input::PalexConfig& config) {
    const std::string utf8_path = config.util_output_path + "/utf8";

    templates::write_template_to_file(cpp_utf8_source, utf8_path + ".cpp", templates::EMPTY_COMPLETER);
    std::cout << "Generating file " << utf8_path << ".cpp..." << std::endl;
    templates::write_template_to_file(cpp_utf8_header, utf8_path + ".h", templates::EMPTY_COMPLETER);
    std::cout << "Generating file " << utf8_path << ".h..." << std::endl;
}

void code_gen::cpp::complete_lexer_header(
    std::ostream& output, 
    const std::string_view tag,
    const std::string& lexer_name, 
    const input::PalexConfig& config, 
    const TokenInfos& tokens
) {
    if (tag == "TOKEN_TYPE_ENUM") {
        output << "UNDEFINED,\n"
               << "            END_OF_FILE";

        for (const std::string& token : tokens.tokens) {
            output << ",\n            " << token;
        }

        for (const std::string& ignored_token : tokens.ignored_tokens) {
            output << ",\n            " << ignored_token;
        }
    } else {
        if (complete_generic_lexer_tags(output, tag, lexer_name, config)) {
            return;
        }
        complete_fallback_header_tags(output, tag);
    }
}

void code_gen::cpp::complete_lexer_source(
    std::ostream& output, 
    const std::string_view tag, 
    const std::string& lexer_name,
    const input::PalexConfig& config, 
    const TokenInfos& tokens, 
    const lexer_generator::LexerAutomaton_t& dfa
) {
    if (tag == "TOKEN_TYPE_COUNT") {
        constexpr size_t RESERVED_TOKEN_COUNT = 2;
        output << (tokens.tokens.size() + tokens.ignored_tokens.size() + RESERVED_TOKEN_COUNT);
    } else if (tag == "TOKEN_TYPE_STRINGS") {
        output << "\"UNDEFINED\",\n"
               << "    \"END_OF_FILE\"";

        for (const std::string& token : tokens.tokens) {
            output << ",\n    \"" << token << '"';
        }

        for (const std::string& ignored_token : tokens.ignored_tokens) {
            output << ",\n    \"" << ignored_token << '"';
        }
    } else if (tag == "LAST_NORMAL_TOKEN") {
        if (tokens.tokens.empty()) {
            output << "END_OF_FILE";
        } else {
            output << tokens.tokens.back();
        }
    } else if (tag == "STATE_MACHINE") {
        write_state_machine(output, dfa, config);
    } else if (tag == "PRE_STATE_MACHINE") {
        output << "const CharacterPosition token_start = this->curr_position;\n"
               << "    size_t state = 0;\n"
               << "    std::u32string identifier = U\"\";\n"
               << "    char32_t curr = this->get_char();\n\n"
               << "    if (this->end()) {\n"
               << "        return Token{Token::TokenType::END_OF_FILE, U\"\", token_start};\n"
               << "    }";
    } else if (tag == "UTF8_LIB_PATH") {
        output << 
            std::filesystem::relative(
                std::filesystem::absolute(config.util_output_path),
                std::filesystem::absolute(config.output_path)
            ).string()
        ;
    } else {
        if (complete_generic_lexer_tags(output, tag, lexer_name, config)) {
            return;
        }
        complete_fallback_source_tags(output, tag, lexer_name, config);
    }
}

void code_gen::cpp::write_state_machine(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const input::PalexConfig& config) {
    output << "while (true) {\n"
           << "        this->cache.push(curr);\n\n"
           << "        switch(state) {\n";
        
    for (const auto& state : dfa.get_states()) {
        write_state(output, dfa, config, state.first);
    }
    write_error_state(output, config);
    
    output << "        }\n\n"
           << "        identifier += this->cache.top();\n"
           << "        this->curr_position.advance(this->cache.top());\n"
           << "        this->cache.pop();\n"
           << "        curr = this->get_char();\n"
           << "    }";
}

void code_gen::cpp::write_state(
    std::ostream& output, 
    const lexer_generator::LexerAutomaton_t& dfa, 
    const input::PalexConfig& config,
    const lexer_generator::LexerAutomaton_t::StateID_t to_write
) {
    output << "            case " << to_write << ":\n";

    if (!dfa.has_outgoing_connections(to_write)) {
        assert(!dfa.get_state(to_write).empty() && "Found DFA with empty leaf node!");
        
        output << "                return Token{Token::TokenType::" << dfa.get_state(to_write) << ", identifier, token_start};\n";
    } else {
        if (!dfa.get_state(to_write).empty() && config.lexer_fallback) {
            output << "                this->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::" << dfa.get_state(to_write) << "};\n";
        }

        write_state_transition_table(output, dfa, to_write);
        
        output << "                break;\n";
    }
}

void code_gen::cpp::write_error_state(std::ostream& output, const input::PalexConfig& config) {
    output << "            case ERROR_STATE:\n";
    
    if (config.lexer_fallback) {
        output << "                return this->try_restore_fallback(identifier, token_start);\n";
    } else {
        output << "                return Token{Token::TokenType::UNDEFINED, identifier, token_start};\n";   
    }
}

void code_gen::cpp::write_state_transition_table(
    std::ostream& output, 
    const lexer_generator::LexerAutomaton_t& dfa, 
    const lexer_generator::LexerAutomaton_t::StateID_t to_write
) {
    output << "                switch(curr) {\n";

    for (const lexer_generator::LexerAutomaton_t::ConnectionID_t conn : dfa.get_outgoing_connection_ids(to_write)) {
        assert(dfa.get_connection(conn).value.has_value() && "Found epsilon connection in DFA!");

        for (const regex::CharRange& range : dfa.get_connection(conn).value.value().get_ranges()) {
            output << "                    case " << (size_t)range.start;
            
            if (!range.is_single_char()) {
                output << " ... " << (size_t)range.end;
            }

            output << ":\n";
        }

        output << "                        state = " << dfa.get_connection(conn).target << ";\n"
               << "                        break;\n";
    }

    output << "                    default:\n";

    if (dfa.get_state(to_write).empty()) {
        output << "                        state = ERROR_STATE;\n"
                << "                        break;\n";
    } else {
        output << "                        return Token{Token::TokenType::" << dfa.get_state(to_write) << ", identifier, token_start};\n";
    }

    output << "                }\n";
}