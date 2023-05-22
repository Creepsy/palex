#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "input/cmd_arguments.h"
#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "util/palex_except.h"

#include "lexer_generator/code_gen/lexer_generation.h"

bool process_rule_file(const std::string& rule_file_path, const input::PalexConfig& config) noexcept;

int main(int argc, char* argv[]) {
    try {
        const input::PalexConfig config = input::parse_config_from_args(argc, argv); 
        if (config.language == input::Language::NONE) {
            throw palex_except::ValidationError("No target language supplied!");
        }
        for (const std::string& rule_file_path : config.rule_files) {
            process_rule_file(rule_file_path, config);
        }
    } catch (const std::exception& err) {
        std::cerr << "Fatal error: " << err.what() << std::endl;
        std::cerr << "Exiting program..." << std::endl;
        return 1;
    }
    return 0;
}

bool process_rule_file(const std::string& rule_file_path, const input::PalexConfig& config) noexcept {
    std::cout << "Processing rule file '" << rule_file_path << "'..." << std::endl;
    const std::string rule_name = std::filesystem::path(rule_file_path).stem().string() + "Lexer"; // TODO: cheking for has_stem?
    std::ifstream rule_file(rule_file_path);
    if (!rule_file.is_open()) {
        std::cerr << "Error: Unable to open rule file '" << rule_file_path << "'!" << std::endl;
        return false;
    }
    input::PalexRuleLexer lexer(rule_file);
    input::PalexRuleParser parser(lexer);
    try {
        const input::PalexRules& palex_rules = parser.parse_all_rules();
        if (config.generate_lexer) {
            code_gen::generate_lexer(rule_name, palex_rules.token_definitions, config);
        }
    } catch (const std::exception& err) {
        rule_file.close();
        std::cerr << "Error: " << err.what() << std::endl;
        return false;
    }
    rule_file.close();
    std::cout << "Processed rule file '" << rule_file_path << "' with success!" << std::endl;
    return true;
}