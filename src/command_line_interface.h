#pragma once

#include <functional>
#include <string_view>

#include "input/cmd_arguments.h"

namespace cli {
    using ProcessRuleFileFunc_t = std::function<bool(const std::string_view, const std::string_view, const input::PalexConfig)>;
    int process_args(const int argc, const char* argv[], const ProcessRuleFileFunc_t& process_rule_file);
}