#include <string_view>
#include <iostream>
#include <string>
#include <functional>

#include "bootstrap/BootstrapLexer.h"
#include "input/PalexRuleParser.h"

#include "lexer_generator/code_gen/lexer_generation.h"

#include "command_line_interface.h"

bool process_rule_file(const std::string_view& module_name, const std::string_view file_contents, const input::PalexConfig& config);

int main(int argc, char* argv[]) {
    return cli::process_args(argc, (const char**)argv, process_rule_file);
}

bool process_rule_file(const std::string_view& module_name, const std::string_view file_contents, const input::PalexConfig& config) {
    bootstrap::BootstrapLexer lexer(file_contents.data());
    input::PalexRuleParser parser(
        std::bind(&bootstrap::BootstrapLexer::next_unignored_token, &lexer),
        std::bind(&bootstrap::BootstrapLexer::get_token, &lexer)
    );
    try {
        const input::PalexRules& palex_rules = parser.parse_all_rules();
        if (config.generate_lexer) {
            code_gen::generate_lexer(std::string(module_name), palex_rules.token_definitions, config);
        }
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return false;
    }
    return true;
}