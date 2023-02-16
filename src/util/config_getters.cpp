#include "config_getters.h"

#include "palex_except.h"

// helper functions
void throw_invalid_tag_error(const std::string& object_name, const std::string& invalid_tag, const std::string& expected_type);

void throw_invalid_tag_error(const std::string& object_name, const std::string& invalid_tag, const std::string& expected_type) {
    throw palex_except::ParserError(
        "In object '" + object_name + "': The config tag '" + invalid_tag + "' is missing or of invalid type (Expected " + expected_type + ")!"
    );
}



void config::require_string(const nlohmann::json& json_config, const std::string& tag, std::string& output, const std::string& object_name) {
    if (!optional_string(json_config, tag, output)) {
        throw_invalid_tag_error(object_name, tag, "string");
    }
}

bool config::optional_string(const nlohmann::json& json_config, const std::string& tag, std::string& output) {
    if (json_config.contains(tag) && json_config.at(tag).is_string()) {
        json_config.at(tag).get_to(output);
    
        return true;
    }

    return false;
}


void config::require_bool(const nlohmann::json& json_config, const std::string& tag, bool& output, const std::string& object_name) {
    if (!optional_bool(json_config, tag, output)) {
        throw_invalid_tag_error(object_name, tag, "bool");
    }
}

bool config::optional_bool(const nlohmann::json& json_config, const std::string& tag, bool& output) {
    if (json_config.contains(tag) && json_config.at(tag).is_boolean()) {
        json_config.at(tag).get_to(output);
    
        return true;
    }

    return false;

}

