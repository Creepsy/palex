#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cassert>

#include "input/cmd_arguments.h"
#include "input/PalexRuleLexer.h"
#include "input/PalexRuleParser.h"

#include "util/palex_except.h"

#include "lexer_generator/code_gen/lexer_generation.h"

void print_help_page(const std::string& program_name);
void print_version_number();
bool process_rule_file(const std::string& rule_file_path, const input::PalexConfig& config) noexcept;

int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "No arguments supplied! Use '" << argv[0] << " --help' to show all available arguments." << std::endl;
        return 1;
    }
    assert(argc > 1 && "BUG: Check for no arguments didn't exit the program!");
    if (std::string(argv[1]) == "--help") {
        print_help_page(argv[0]);
        return 0;
    }
    if (std::string(argv[1]) == "--version") {
        print_version_number();
        return 0;
    }
    try {
        const input::PalexConfig config = input::parse_config_from_args(argc, (const char**)argv); 
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

void print_help_page(const std::string& program_name) {
    std::cout << "Usage:\n"
              << "  " << program_name << " [FILE]... [OPTION]... [FLAG]...  Runs the generator with the specified arguments.\n"
              << "  " << program_name << " --version                        Prints the version number of palex.\n"
              << "  " << program_name << " --help                           Prints this help page.\n\n"
              << "Options:\n"
              << "  -output-path <path>         Output folder for lexer and parser files (default: current directory).\n"
              << "  -util-path <path>           Output (and import) folder for util files (default: current directory).\n"
              << "  -lang <C++|CPP>             The target programming language (mandatory).\n"
              << "  -parser-type <LR|LALR>      The type of the generated parsers (mandatory when --parser flag is set).\n"
              << "  -lookahead <uint>           Lookahead token count (integer >= 0).\n"
              << "  -module-name <name>         The name of the module/namespace of the generated code (default: palex).\n\n"
              << "Flags:\n"
              << "  --lexer                     Enable lexer generation.\n"
              << "  --parser                    Enable parser generation.\n"
              << "  --util                      Enable generation of utility files.\n"
              << "  --fallback                  Enables token fallback for lexers. \n"
    ;         
}

void print_version_number() {
    std::cout << "palex v" << PALEX_VERSION << std::endl; 
}

bool process_rule_file(const std::string& rule_file_path, const input::PalexConfig& config) noexcept {
    std::cout << "Processing rule file '" << rule_file_path << "'..." << std::endl;
    if (!std::filesystem::path(rule_file_path).has_stem()) {
        std::cerr << "Error: Unable to get the name of the rule file '" << rule_file_path << "'!";
        return false;
    }
    const std::string rule_name = std::filesystem::path(rule_file_path).stem().string() + "Lexer";
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