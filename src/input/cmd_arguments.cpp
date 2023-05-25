#include "cmd_arguments.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cassert>

#include "util/palex_except.h"

namespace input {
    // helper functions
    std::vector<std::string> convert_args(const int argc, const char** argv);
    std::string lower(const std::string& to_lower);
    bool is_flag(const std::string& to_check);
    bool is_option(const std::string& to_check);
    void parse_flag(const std::string& flag, PalexConfig& target);
    void parse_option(const std::string& type, const std::string& parameter, PalexConfig& target);
    void parse_lang(const std::string& language, PalexConfig& target);
    void parse_parser_type(const std::string& parser_type, PalexConfig& target);
    void parse_lookahead(const std::string& lookahead, PalexConfig& target);
    void parse_module_name(const std::string& module_name, PalexConfig& target);

    std::vector<std::string> convert_args(const int argc, const char** argv) {
        std::vector<std::string> arguments(argc, "");
        for (int curr_arg = 0; curr_arg < argc; curr_arg++) {
            arguments[curr_arg] = argv[curr_arg];
        }
        return arguments;
    }

    std::string lower(const std::string& to_lower) {
        std::string lower_case_str(to_lower.size(), '\0');
        std::transform(
            to_lower.begin(), 
            to_lower.end(), 
            lower_case_str.begin(), 
            [](const char to_convert) -> char { 
                return (char)std::tolower(to_convert); 
            }
        );
        return lower_case_str;
    }

    bool is_flag(const std::string& to_check) {
        return to_check.size() >= 3 && to_check.substr(0, 2) == "--";   
    }

    bool is_option(const std::string& to_check) {
        return to_check.size() >= 2 && !is_flag(to_check) && *to_check.begin() == '-';
    }

    void parse_flag(const std::string& flag, PalexConfig& target) {
        if (flag == "lexer") {
            target.generate_lexer = true;
        } else if (flag == "parser") {
            target.generate_parser = true;
        } else if (flag == "util") {
            target.generate_util = true;
        } else if (flag == "fallback") {
            target.lexer_fallback = true;
        } else {
            throw palex_except::ParserError("Unknown flag '--" + flag + "' supplied!");
        }
    }

    void parse_option(const std::string& type, const std::string& parameter, PalexConfig& target) {
        if (type == "output-path") {
            target.output_path = parameter;
        } else if (type == "util-path") {
            target.util_output_path = parameter;
        } else if (type == "lang") {
            parse_lang(parameter, target);
        } else if (type == "parser-type") {
            parse_parser_type(parameter, target);
        } else if (type == "lookahead") {
            parse_lookahead(parameter, target);
        } else if (type == "module-name") {
            parse_module_name(parameter, target);
        } else {
            throw palex_except::ParserError("Unknown option '-" + type + " " + parameter + "' supplied!");
        }
    }

    void parse_lang(const std::string& language, PalexConfig& target) {
        const std::string language_lowercase = lower(language);
        if (language_lowercase == "c++" || language_lowercase == "cpp") {
            target.language = Language::CPP;
        } else {
            throw palex_except::ParserError("Unknown language '" + language + "' supplied to lang option!");
        }
    }

    void parse_parser_type(const std::string& parser_type, PalexConfig& target) {
        const std::string parser_type_lowercase = lower(parser_type);
        if (parser_type_lowercase == "lalr") {
            target.parser_type = ParserType::LALR;
        } else if (parser_type_lowercase == "lr") {
            target.parser_type = ParserType::LR;
        } else {
            throw palex_except::ParserError("Unknown parser type '" + parser_type + "' supplied to parser type option!");
        }
    }

    void parse_lookahead(const std::string& lookahead, PalexConfig& target) {
        if (!std::all_of(lookahead.begin(), lookahead.end(), [](const char to_check) -> bool { return std::isdigit(to_check); })) {
            throw palex_except::ParserError("Invalid number '" + lookahead + "' supplied to lookahead option!");
        }
        try {
            target.lookahead = std::stoull(lookahead);
        } catch (const std::invalid_argument& invalid_arg_err) {
            assert(false && "BUG: Validity check should already have occured before!");
        } catch (const std::out_of_range& out_of_range_err) {
            throw palex_except::ParserError("Too big number '" + lookahead + "' supplied to lookahead option!");
        }
    }

    void parse_module_name(const std::string& module_name, PalexConfig& target) {
        const bool is_valid_identifier = 
            !module_name.empty() && 
            (module_name.front() == '_' || std::isalpha(module_name.front())) && 
            std::all_of(
                module_name.begin(), 
                module_name.end(), 
                [](const char to_check) -> bool { 
                    return std::isalnum(to_check) || to_check == '_'; 
                }
            )
        ;
        if (!is_valid_identifier) {
            throw palex_except::ParserError("'" + module_name + "' is not a valid module name!");
        }
        target.module_name = module_name;
    }

    PalexConfig parse_config_from_args(const int argc, const char** argv) {
        const std::vector<std::string> arguments = convert_args(argc, argv);
        PalexConfig config{};
        for (size_t curr_arg = 1; curr_arg < arguments.size(); curr_arg++) { // skip the first arg as it's the programs name
            if (is_flag(arguments[curr_arg])) {
                parse_flag(arguments[curr_arg].substr(2), config);
            } else if (is_option(arguments[curr_arg])) {
                if (curr_arg == arguments.size() - 1) {
                    throw palex_except::ParserError("No parameter supplied to option '" + arguments[curr_arg] + "'!");
                }
                parse_option(arguments[curr_arg].substr(1), arguments[curr_arg + 1], config);
                curr_arg++;
            } else {
                config.rule_files.push_back(arguments[curr_arg]);
            }
        }

        return config;
    }
}