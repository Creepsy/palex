#include <string_view>
#include <iostream>
#include <string>
#include <functional>

#include "input/PalexLexerAdapter.h"
#include "PalexRuleLexer.h"

#include "input/PalexRuleParser.h"

#include "lexer_generator/code_gen/lexer_generation.h"

#include "parser_generator/shift_reduce_parsers/code_gen/parser_generation.h"

#include "command_line_interface.h"

bool process_rule_file(const std::string_view& module_name, const std::string_view file_contents, const input::PalexConfig& config);

int main(int argc, char* argv[]) {
    return cli::process_args(argc, (const char**)argv, process_rule_file);
}

bool process_rule_file(const std::string_view& module_name, const std::string_view file_contents, const input::PalexConfig& config) {
    input::PalexRuleLexer lexer(file_contents.data());
    input::PalexLexerAdapter adapter(lexer);
    input::PalexRuleParser parser(
        std::bind(&input::PalexLexerAdapter::next_token, &adapter),
        std::bind(&input::PalexLexerAdapter::get_token, &adapter)
    );
    try {
        const input::PalexRules& palex_rules = parser.parse_all_rules();
        if (config.generate_lexer) {
            code_gen::generate_lexer(std::string(module_name), palex_rules.token_definitions, config);
        }
        if (config.generate_parser) {
            parser_generator::shift_reduce_parsers::code_gen::generate_parser(std::string(module_name), palex_rules.productions, config);
        }
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return false;
    }
    return true;
}