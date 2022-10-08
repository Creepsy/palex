#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <stdexcept>

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
    generate_lexer_header(dfa, config, tokens);
    generate_lexer_source(dfa, config, tokens);

    if(config.create_utf8_lib) {
        generate_utf8_lib(config);
    }
}

void code_gen::cpp::generate_lexer_header(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens) {
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

void code_gen::cpp::generate_lexer_source(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string source_file_path = config.lexer_path + "/" + config.lexer_name + ".cpp";

    std::ofstream source_file(source_file_path);

    if(!source_file.is_open()) {
        throw std::runtime_error("Unable to open file '" + source_file_path + "'!");
    }

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_source, _1, _2, config, tokens);

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
            output << ",\n\t\t\t" << unicode::to_utf8(token);
        }

        for(const std::u32string& ignored_token : tokens.ignored_tokens) {
            output << ",\n\t\t\t" << unicode::to_utf8(ignored_token);
        }
    } else if(tag == "FALLBACK") {
        //TODO: currently unimplemented
    }
}

void code_gen::cpp::complete_lexer_source(std::ostream& output, const std::string_view tag, const CppLexerConfig& config, const TokenInfos& tokens) {
    if(tag == "LEXER_NAME") {
        output << config.lexer_name;
    } else if(tag == "TOKEN_TYPE_COUNT") {
        constexpr size_t RESERVED_TOKEN_COUNT = 2;
        output << (tokens.tokens.size() + tokens.ignored_tokens.size() + RESERVED_TOKEN_COUNT);
    } else if(tag == "TOKEN_TYPE_STRINGS") {
        output << "\"UNDEFINED\",\n"
               << "\t\"END_OF_FILE\"";

        for(const std::u32string& token : tokens.tokens) {
            output << ",\n\t\"" << unicode::to_utf8(token) << '"';
        }

        for(const std::u32string& ignored_token : tokens.ignored_tokens) {
            output << ",\n\t\"" << unicode::to_utf8(ignored_token) << '"';
        }
    } else if(tag == "LEXER_NAMESPACE") {
        output << config.lexer_namespace;
    } else if(tag == "LAST_NORMAL_TOKEN") {
        if(tokens.tokens.empty()) {
            output << "END_OF_FILE";
        } else {
            output << unicode::to_utf8(tokens.tokens.back());
        }
    }
}
