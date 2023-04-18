#pragma once

#include <vector>
#include <set>

#include "ParserProductionParser.h"

namespace parser_generator {
    void validate_productions(const std::vector<Production>& to_check);
    void check_for_missing_productions(const std::vector<Production>& to_check);
    void check_for_duplicate_productions(const std::vector<Production>& to_check);
    void check_for_entry(const std::vector<Production>& to_check);
}