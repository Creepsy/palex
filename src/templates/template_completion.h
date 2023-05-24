#pragma once

#include <ostream>
#include <functional>
#include <string_view>
#include <string>
#include <map>

namespace templates {
    using TemplateCompleter_t = std::function<void (std::ostream&)>;

    extern TemplateCompleter_t EMPTY_COMPLETER;

    TemplateCompleter_t constant_completer(const std::string_view& completer_output);
    TemplateCompleter_t conditional_completer(const bool enable_flag, const std::string_view& completer_output);
    void write_template_to_stream(
        const char* const to_write, 
        std::ostream& output, 
        const std::map<std::string_view, TemplateCompleter_t>& completers
    );
    void write_template_to_file(const char* const to_write, const std::string& output_path, const std::map<std::string_view, TemplateCompleter_t>& completers);
}