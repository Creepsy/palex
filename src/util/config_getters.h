#pragma once

#include <string>

#include <json.h>

namespace config {
    void require_string(const nlohmann::json& json_config, const std::string& tag, std::string& output, const std::string& object_name);
    bool optional_string(const nlohmann::json& json_config, const std::string& tag, std::string& output);

    void require_bool(const nlohmann::json& json_config, const std::string& tag, bool& output, const std::string& object_name);
    bool optional_bool(const nlohmann::json& json_config, const std::string& tag, bool& output);
}