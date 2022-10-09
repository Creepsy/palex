#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cassert>

#include "templates/template_completion.h"

#include "util/unicode.h"

#include "cpp_lexer_source.h"
#include "cpp_lexer_header.h"
#include "cpp_utf8_source.h"
#include "cpp_utf8_header.h"

code_gen::cpp::CppLexerConfig code_gen::cpp::create_lexer_config_from_json(const nlohmann::json& json_config) {
    return CppLexerConfig{}; //TODO
}

void code_gen::cpp::generate_lexer_files(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens) {
    generate_lexer_header(config, tokens);
    generate_lexer_source(config, tokens, dfa);

    if(config.create_utf8_lib) {
        generate_utf8_lib(config);
    }
}

void code_gen::cpp::generate_lexer_header(const CppLexerConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.lexer_path + "/" + config.lexer_name + ".h";

    std::ofstream header_file(header_file_path);

    if(!header_file.is_open()) {
        throw std::runtime_error("Unable to open file '" + header_file_path + "'!");
    }

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_header, _1, _2, config, tokens);

    templates::write_template_to_stream(cpp_lexer_header, header_file, completer);

    header_file.close();
}

void code_gen::cpp::generate_lexer_source(const CppLexerConfig& config, const TokenInfos& tokens, const lexer_generator::LexerAutomaton_t& dfa) {
    using namespace std::placeholders;

    const std::string source_file_path = config.lexer_path + "/" + config.lexer_name + ".cpp";

    std::ofstream source_file(source_file_path);

    if(!source_file.is_open()) {
        throw std::runtime_error("Unable to open file '" + source_file_path + "'!");
    }

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_source, _1, _2, config, tokens, dfa);

    templates::write_template_to_stream(cpp_lexer_source, source_file, completer);

    source_file.close();
}

void code_gen::cpp::generate_utf8_lib(const CppLexerConfig& config) {
    const std::string utf8_path = config.utf8_lib_path + "/utf8";

    std::ofstream source_file(utf8_path + ".cpp");
    templates::write_template_to_stream(cpp_utf8_source, source_file, templates::EMPTY_COMPLETER);
    source_file.close();

    std::ofstream header_file(utf8_path + ".h");
    templates::write_template_to_stream(cpp_utf8_header, header_file, templates::EMPTY_COMPLETER);
    header_file.close();
}

void code_gen::cpp::complete_lexer_header(std::ostream& output, const std::string_view tag, const CppLexerConfig& config, const TokenInfos& tokens) {
    if(tag == "LEXER_NAMESPACE") {
        output << config.lexer_namespace;
    } else if(tag == "LEXER_NAME") {
        output << config.lexer_name;
    } else if(tag == "TOKEN_TYPE_ENUM") {
        output << "UNDEFINED,\n"
               << "\t\t\tEND_OF_FILE";

        for(const std::u32string& token : tokens.tokens) {
            output << ",\n\t\t\t" << token;
        }

        for(const std::u32string& ignored_token : tokens.ignored_tokens) {
            output << ",\n\t\t\t" << ignored_token;
        }
    } else if(tag == "FALLBACK_CACHE" && config.token_fallback) {
        output << "std::optional<Fallback> fallback;";
    } else if(tag == "FALLBACK_INCLUDES" && config.token_fallback) {
        output << "#include <optional>\n"
               << "#include <utility>";
    } else if(tag == "FALLBACK_TYPES" && config.token_fallback) {
        output << "struct Fallback {\n"
               << "\t\t\t\tsize_t token_length;\n"
               << "\t\t\t\tCharacterPosition next_token_position;\n"
               << "\t\t\t\tToken::TokenType type;\n"
               << "\t\t\t};\n\n"
               << "\t\t\tToken try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start);";
    }
}

void code_gen::cpp::complete_lexer_source(
    std::ostream& output, 
    const std::string_view tag, 
    const CppLexerConfig& config, 
    const TokenInfos& tokens, 
    const lexer_generator::LexerAutomaton_t& dfa
) {
    if(tag == "LEXER_NAME") {
        output << config.lexer_name;
    } else if(tag == "TOKEN_TYPE_COUNT") {
        constexpr size_t RESERVED_TOKEN_COUNT = 2;
        output << (tokens.tokens.size() + tokens.ignored_tokens.size() + RESERVED_TOKEN_COUNT);
    } else if(tag == "TOKEN_TYPE_STRINGS") {
        output << "\"UNDEFINED\",\n"
               << "\t\"END_OF_FILE\"";

        for(const std::u32string& token : tokens.tokens) {
            output << ",\n\t\"" << token << '"';
        }

        for(const std::u32string& ignored_token : tokens.ignored_tokens) {
            output << ",\n\t\"" << ignored_token << '"';
        }
    } else if(tag == "LEXER_NAMESPACE") {
        output << config.lexer_namespace;
    } else if(tag == "LAST_NORMAL_TOKEN") {
        if(tokens.tokens.empty()) {
            output << "END_OF_FILE";
        } else {
            output << tokens.tokens.back();
        }
    } else if(tag == "STATE_MACHINE") {
        write_state_machine(output, dfa, config);
    } else if(tag == "PRE_STATE_MACHINE") {
        if(config.token_fallback) {
            output << "this->fallback = std::nullopt;\n\t";
        }

        output << "const CharacterPosition token_start = this->curr_position;\n"
               << "\tsize_t state = 0;\n"
               << "\tstd::u32string identifier = U\"\";\n"
               << "\tchar32_t curr = this->get_char();\n\n"
               << "\tif(this->end()) {\n"
               << "\t\treturn Token{Token::TokenType::END_OF_FILE, U\"\", token_start};\n"
               << "\t}";
    } else if(tag == "UTF8_LIB_PATH") {
        output << config.utf8_lib_path;
    } else if(tag == "RESTORE_FALLBACK_FUNC" && config.token_fallback) {
        output << config.lexer_namespace << "::Token " << config.lexer_namespace << "::" << config.lexer_name
               << "::try_restore_fallback(std::u32string& token_identifier, const CharacterPosition token_start) {\n"
               << "\tif(!this->fallback.has_value()) return Token{Token::TokenType::UNDEFINED, token_identifier, token_start};\n\n"
               << "\twhile(token_identifier.size() > this->fallback.value().token_length) {\n"
               << "\t\tthis->cache.push(token_identifier.back());\n"
               << "\t\ttoken_identifier.pop_back();\n"
               << "\t}\n\n"
               << "\tthis->curr_position = this->fallback.value().next_token_position;\n\n"
               << "\treturn Token{this->fallback.value().type, token_identifier, token_start};\n"
               << "}";
    }
}

void code_gen::cpp::write_state_machine(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config) {
    output << "while(true) {\n"
           << "\t\tthis->cache.push(curr);\n\n"
           << "\t\tswitch(state) {\n";
        
    write_states(output, dfa, config);

    output << "\t\t\tcase (size_t)-1: // error state\n";
    
    if(config.token_fallback) {
        output << "\t\t\t\treturn this->try_restore_fallback(identifier, token_start);\n";
    } else {
        output << "\t\t\t\treturn Token{Token::TokenType::UNDEFINED, identifier, token_start};\n";   
    }
    
    output << "\t\t}\n\n"
           << "\t\tidentifier += this->cache.top();\n"
           << "\t\tthis->curr_position.advance(this->cache.top());\n"
           << "\t\tthis->cache.pop();\n"
           << "\t\tcurr = this->get_char();\n"
           << "\t}";
}

void code_gen::cpp::write_states(std::ostream& output, const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config) {
    for(const auto& state : dfa.get_states()) {
        output << "\t\t\tcase " << state.first << ":\n";
        write_state(output, dfa, config, state.first);
        output << "\t\t\t\tbreak;\n";
    }
}

void code_gen::cpp::write_state(
    std::ostream& output, 
    const lexer_generator::LexerAutomaton_t& dfa, 
    const CppLexerConfig& config,
    const lexer_generator::LexerAutomaton_t::StateID_t to_write
) {
    const std::vector<lexer_generator::LexerAutomaton_t::ConnectionID_t> outgoing_connections = dfa.get_outgoing_connection_ids(to_write);

    if(outgoing_connections.empty()) {
        assert(!dfa.get_state(to_write).empty() && "Found DFA with empty leaf node!");
        
        output << "\t\t\t\treturn Token{Token::TokenType::" << dfa.get_state(to_write) << ", identifier, token_start};\n";
    } else {
        if(!dfa.get_state(to_write).empty() && config.token_fallback) {
            output << "\t\t\t\tthis->fallback = Fallback{identifier.size(), this->curr_position, Token::TokenType::" << dfa.get_state(to_write) << "};\n";
        }

        output << "\t\t\t\tswitch(curr) {\n";

        for(const lexer_generator::LexerAutomaton_t::ConnectionID_t conn : outgoing_connections) {
            assert(dfa.get_connection(conn).value.has_value() && "Found epsilon connection in DFA!");

            for(const regex::CharRange& range : dfa.get_connection(conn).value.value().get_ranges()) {
                output << "\t\t\t\t\tcase " << (size_t)range.start;
                
                if(!range.is_single_char()) {
                    output << " ... " << (size_t)range.end;
                }

                output << ":\n";
            }

            output << "\t\t\t\t\t\tstate = " << dfa.get_connection(conn).target << ";\n"
                   << "\t\t\t\t\t\tbreak;\n";
        }

        output << "\t\t\t\t\tdefault:\n";

        if(dfa.get_state(to_write).empty()) {
            output << "\t\t\t\t\t\tstate = (size_t)-1;\n"
                   << "\t\t\t\t\t\tbreak;\n";

        } else {
            output << "\t\t\t\t\t\treturn Token{Token::TokenType::" << dfa.get_state(to_write) << ", identifier, token_start};\n";
        }

        output << "\t\t\t\t}\n";
    }
}