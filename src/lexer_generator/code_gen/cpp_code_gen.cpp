#include "cpp_code_gen.h"

#include <functional>
#include <fstream>
#include <stdexcept>

#include "templates/template_completion.h"

#include "util/unicode.h"

#include "cpp_lexer_source.h"
#include "cpp_lexer_header.h"

code_gen::cpp::CppLexerConfig code_gen::cpp::create_lexer_config_from_json(const nlohmann::json& json_config) {
    return CppLexerConfig{}; //TODO
}

void code_gen::cpp::generate_lexer_files(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens) {
    generate_lexer_header(dfa, config, tokens);
    generate_lexer_source(dfa, config);
    generate_unicode_lib();
}

void code_gen::cpp::generate_lexer_header(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config, const TokenInfos& tokens) {
    using namespace std::placeholders;

    const std::string header_file_path = config.output_path + "/" + config.lexer_name + ".h";

    std::ofstream header_file(header_file_path);

    if(!header_file.is_open()) {
        throw std::runtime_error("Unable to open file '" + header_file_path + "'!");
    }

    templates::TemplateCompleter_t completer = std::bind(complete_lexer_header, _1, _2, config, tokens);

    templates::write_template_to_stream(cpp_lexer_header, header_file, completer);

    header_file.close();
}

void code_gen::cpp::generate_lexer_source(const lexer_generator::LexerAutomaton_t& dfa, const CppLexerConfig& config) {
    //TODO
}

void code_gen::cpp::generate_unicode_lib() {
    //TODO
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

void code_gen::cpp::complete_lexer_source(std::ostream& output, const std::string_view tag) {
    //TODO
}
