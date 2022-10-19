#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

#include <json.h>

#include "util/config_getters.h"
#include "util/palex_except.h"

#include "lexer_generator/code_gen/lexer_generation.h"

bool load_config(const std::string& config_path, nlohmann::json& parsed_config);

int main(int argc, char* argv[]) {  
    if(argc > 2) {
        std::cerr << "Warning: Ignoring " << argc - 2 << " extra argument(s) from command line!" << std::endl;
    }

    if(argc > 1) {
        std::filesystem::current_path(argv[1]);
    }

    const std::string config_path = std::filesystem::current_path().string() + "/palex.cfg";
    nlohmann::json json_config;

    if(!load_config(config_path, json_config)) {
        return 1;
    }

    std::string target_language;
    try {
        config::require_string(json_config, "language", target_language, "config");
    } catch(const palex_except::ParserError& e) {
        std::cerr << e.what() << std::endl;

        return 1;
    }

    if(json_config.contains("lexers")) {
        if(!json_config.at("lexers").is_object()) {
            std::cerr << "Invalid config format! The tag 'lexers' is of invalid type (Expected map)!" << std::endl;

            return 1;
        }

        for(const std::pair<std::string, nlohmann::json>& lexer_config : json_config.at("lexers").get<nlohmann::json::object_t>()) {
            code_gen::generate_lexer(lexer_config.first, lexer_config.second, target_language);
        }
    }

    return 0;
}

bool load_config(const std::string& config_path, nlohmann::json& parsed_config) {
    std::ifstream config_file;
    config_file.open(config_path);

    if(!config_file.is_open()) {
        std::cerr << "Unable to open config file '" + config_path + "'!" << std::endl;
        
        return false;
    }

    try {
        parsed_config = nlohmann::json::parse(config_file);
    } catch(const nlohmann::json::parse_error& e) {
        config_file.close();
        std::cerr << "Invalid config format! Parsing concluded in error: " << e.what() << std::endl;

        return false;
    }
    config_file.close();

    if(!parsed_config.is_object()) {
        std::cerr << "Invalid config format! Expected a map on the top level of the config file!" << std::endl;

        return false;
    }

    return true;
}