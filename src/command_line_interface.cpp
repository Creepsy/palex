#include "command_line_interface.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cassert>
#include <optional>
#include <sstream>

#include "input/cmd_arguments.h"

#include "util/palex_except.h"

struct RuleFileInfo {
    std::string module_name;
    std::string file_contents;
};

// helper functions
void print_help_page(const std::string& program_name);
void print_version_number();
std::optional<RuleFileInfo> load_rule_file(const std::string_view file_path);
std::string load_stream(const std::istream& to_load);

namespace cli {
    int process_args(const int argc, const char* argv[], const ProcessRuleFileFunc_t& process_rule_file) {
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
                std::cout << "Processing rule file '" << rule_file_path << "'..." << std::endl;
                const std::optional<RuleFileInfo> rule_file_info = load_rule_file(rule_file_path);
                if (!rule_file_info.has_value()) {
                    continue; // problem occured on opening rule file. The "load_rule_file" function is responsible for error logging
                }
                if (process_rule_file(rule_file_info.value().module_name, rule_file_info.value().file_contents, config)) {
                    std::cout << "Processed rule file '" << rule_file_path << "' with success!" << std::endl;
                }

            }
        } catch (const std::exception& err) {
            std::cerr << "Fatal error: " << err.what() << std::endl;
            std::cerr << "Exiting program..." << std::endl;
            return 1;
        }
        return 0;
    }
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

std::optional<RuleFileInfo> load_rule_file(const std::string_view file_path) {
    if (!std::filesystem::path(file_path).has_stem()) {
        std::cerr << "Error: Unable to get the name of the rule file '" << file_path << "'!";
        return std::nullopt;
    }
    const std::string module_name = std::filesystem::path(file_path).stem().string();
    std::ifstream rule_file{std::string(file_path)};
    if (!rule_file.is_open()) {
        std::cerr << "Error: Unable to open the file '" << file_path << "'!" << std::endl;
        return std::nullopt;
    }
    std::string rule_file_contents = load_stream(rule_file);
    rule_file.close();
    return RuleFileInfo{module_name, rule_file_contents};
}


std::string load_stream(const std::istream& to_load) {
    std::stringstream file_content;
    file_content << to_load.rdbuf();
    return file_content.str();
}