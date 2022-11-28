#pragma once

#include <json.h>

namespace config {
    bool load_config(const std::string& config_path, nlohmann::json& parsed_config);
}