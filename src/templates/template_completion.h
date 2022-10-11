#pragma once

#include <ostream>
#include <functional>
#include <string_view>
#include <string>

namespace templates {
    using TemplateCompleter_t = std::function<void (std::ostream&, const std::string_view)>;

    extern TemplateCompleter_t EMPTY_COMPLETER;

    void write_template_to_stream(const char* const to_write, std::ostream& output, TemplateCompleter_t completer);
    void write_template_to_file(const char* const to_write, const std::string& output_path, TemplateCompleter_t completer);
}