#pragma once

#include <ostream>
#include <functional>
#include <string_view>

namespace templates {
    using TemplateCompleter_t = std::function<void (std::ostream&, const std::string_view)>;

    void write_template_to_stream(const char* const to_write, std::ostream& output, TemplateCompleter_t completer);
}