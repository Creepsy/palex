#include "config_loader.h"

#include <fstream>
#include <iostream>

bool config::load_config(const std::string& config_path, nlohmann::json& parsed_config) {
    std::ifstream config_file;
    config_file.open(config_path);

    if (!config_file.is_open()) {
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

    if (!parsed_config.is_object()) {
        std::cerr << "Invalid config format! Expected a map on the top level of the config file!" << std::endl;

        return false;
    }

    return true;
}