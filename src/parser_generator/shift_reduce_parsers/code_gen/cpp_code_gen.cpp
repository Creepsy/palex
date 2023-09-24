#include "cpp_code_gen.h"

#include <iostream>
#include <stdexcept>
#include <map>
#include <string_view>
#include <sstream>
#include <functional>
#include <set>
#include <variant>
#include <algorithm>

#include "templates/template_completion.h"

#include "util/stream_format.h"
#include "util/Visitor.h"

#include "parser_generator/shift_reduce_parsers/parser_state.h"

#include "cpp_parser_header.h"
#include "cpp_parser_source.h"
#include "cpp_types_header.h"
#include "cpp_types_source.h"
#include "cpp_ast_builder_header.h"

const std::string LOOKAHEAD_FUNCTION_COMPLETION = 
R"(size_t %MODULE_NAMESPACE%::%UNIT_NAME%Parser::get_lookahead_id() const {
    const static std::map<std::array<%UNIT_NAME%Token::TokenType, LOOKAHEAD>, size_t> lookahead_mappings = {
%LOOKAHEAD_MAPPINGS%    };

    std::array<%UNIT_NAME%Token::TokenType, LOOKAHEAD> lookahead_key;
    size_t i = 0;
    for (auto iter = this->lookahead.begin(); iter != this->lookahead.end(); iter++) {
        lookahead_key[i++] = iter->type;
    }
    const auto mapping = lookahead_mappings.find(lookahead_key);
    return (mapping == lookahead_mappings.end()) ? ERROR_STATE : mapping->second; 
})";

// helper functions
void complete_lookahead_function_declaration(const input::PalexConfig& config, std::ostream& output);
void complete_nonterminal_enum(const std::vector<parser_generator::Production>& productions, std::ostream& output);
void complete_init_lookahead(const input::PalexConfig& config, std::ostream& output);
void complete_shift_function(const input::PalexConfig& config, std::ostream& output);
void complete_lookahead_function(
    const parser_generator::shift_reduce_parsers::ParserTable& parser_table, 
    const std::string& unit_name, 
    const input::PalexConfig& config, 
    std::ostream& output
);
void complete_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, std::ostream& output);
void complete_goto_states(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, std::ostream& output);
void complete_parser_table(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, const input::PalexConfig& config, std::ostream& output);
void complete_lookahead_switch(const input::PalexConfig& config, std::ostream& output);
void complete_lookahead_case(
    const parser_generator::shift_reduce_parsers::Lookahead_t& lookahead, 
    const std::string& unit_name,
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t>& mappings,
    std::ostream& output);
void complete_lookahead_printer(const input::PalexConfig& config, std::ostream& output);
void complete_position_printer(const input::PalexConfig& config, std::ostream& output);
std::string upper_case_str(const std::string& to_convert);
std::set<std::string> collect_nonterminal_names(const std::vector<parser_generator::Production>& productions);
std::set<parser_generator::shift_reduce_parsers::Lookahead_t> collect_all_lookaheads(const parser_generator::shift_reduce_parsers::ParserTable& parser_table);
std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> create_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table);
std::string create_state_error_message(const parser_generator::shift_reduce_parsers::ParserState& state);
std::string expected_tokens_to_string(const parser_generator::shift_reduce_parsers::ParserState& state, const std::string& unit_name);
void complete_reduce_methods(const std::vector<parser_generator::Production>& productions, std::ostream& output);
void complete_lookahead_to_string(const input::PalexConfig& config, std::ostream& output);

void complete_lookahead_function_declaration(const input::PalexConfig& config, std::ostream& output) {
    if (config.lookahead <= 1) {
        return;
    }
    output << "size_t get_lookahead_id() const;";
}

void complete_nonterminal_enum(const std::vector<parser_generator::Production>& productions, std::ostream& output) {
    output << sfmt::Indentation{2};
    for (const std::string& nonterminal_name : collect_nonterminal_names(productions)) {
        output << upper_case_str(nonterminal_name) << ",\n";
    }
    output << sfmt::Indentation{-2};
}

void complete_init_lookahead(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{2};
    if (config.lookahead <= 1) {
        output << "this->next_token();\n"
                  "this->lookahead = this->current_token();";
    } else {
        output << "while (this->lookahead.size() < LOOKAHEAD) {\n"
                  "    this->next_token();\n"
                  "    this->lookahead.push_back(this->current_token());\n"
                  "}";
    }
    output << sfmt::Indentation{-2};
}

void complete_shift_function(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{2};
    if (config.lookahead <= 1) {
        output << "this->parser_stack.push(ParserStackInfo{next_state, ((size_t)this->lookahead.type << 1) | TERMINAL_SYMBOL_TYPE_FLAG});\n"
                  "this->ast_builder.shift_token(this->lookahead);\n"
                  "this->next_token();\n"
                  "this->lookahead = this->current_token();";
    } else {
        output << "this->parser_stack.push(ParserStackInfo{next_state, ((size_t)this->lookahead.front().type << 1) | TERMINAL_SYMBOL_TYPE_FLAG});\n"
                  "this->ast_builder.shift_token(this->lookahead.front());\n"
                  "this->lookahead.pop_front();\n"
                  "if (this->lookahead.size() < LOOKAHEAD) {\n"
                  "    this->next_token();\n"
                  "    this->lookahead.push_back(this->current_token());\n"
                  "}";
    }
    output << sfmt::Indentation{-2};
}

void complete_lookahead_function(
    const parser_generator::shift_reduce_parsers::ParserTable& parser_table, 
    const std::string& unit_name, 
    const input::PalexConfig& config, 
    std::ostream& output
) {
    using namespace std::placeholders;

    if (config.lookahead <= 1) {
        return;
    }
    const std::map<std::string_view, templates::TemplateCompleter_t> completers = { 
        {"UNIT_NAME", templates::constant_completer(unit_name)},
        {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
        {"LOOKAHEAD_MAPPINGS", std::bind(complete_lookahead_mappings, parser_table, unit_name, _1)}
    };
    templates::write_template_to_stream(LOOKAHEAD_FUNCTION_COMPLETION.c_str(), output, completers);
}

void complete_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, std::ostream& output) {
    output << sfmt::Indentation{2};
    for (const auto& [lookahead, id] : create_lookahead_mappings(parser_table)) {
        output << "{{";
        for (size_t i = 0; i < lookahead.size(); i++) {
            if (i != 0) {
                output << ", ";
            }
            output << unit_name << "Token::TokenType::" << lookahead[i].identifier;
        }
        output << "}, " << id << "},\n";
    }
    output << sfmt::Indentation{-2};
}

void complete_goto_states(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, std::ostream& output) {
    using namespace parser_generator::shift_reduce_parsers;

    output << sfmt::Indentation{3};
    for (size_t id = 0; id < parser_table.get_states().size(); id++) {
        output << "case " << id << ":\n";
        output << sfmt::Indentation{1};
        output << "switch (reduced_production) {\n";
        output << sfmt::Indentation{1};
        for (const Action& action : parser_table.get_states()[id].get_actions()) {
            std::visit(
                Visitor{
                    [&](const Action::GotoParameters& goto_action) {
                        output << "case "<< unit_name << "NonterminalType::" << upper_case_str(goto_action.reduced_symbol.identifier) << ":\n"
                                  "    return " << goto_action.next_state << ";\n";
                    },
                    [](const Action::ReduceParameters& reduce_action) {},
                    [](const Action::ShiftParameters& shift_action) {}
                },
                action.parameters
            );
        }
        output << "default:\n"
                  "    return ERROR_STATE;\n";
        output << sfmt::Indentation{-1};
        output << "}\n";
        output << sfmt::Indentation{-1};
    }
    output << sfmt::Indentation{-3};
}

void complete_parser_table(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const std::string& unit_name, const input::PalexConfig& config, std::ostream& output) {
    using namespace parser_generator::shift_reduce_parsers;
   
    output << sfmt::Indentation{4};
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> mappings = create_lookahead_mappings(parser_table);
    for (size_t id = 0; id < parser_table.get_states().size(); id++) {
        output << "case " << id << ":\n";
        output << sfmt::Indentation{1};
        output << "switch (";
        complete_lookahead_switch(config, output);
        output << ") {\n";
        output << sfmt::Indentation{1};
        bool already_has_default_action = false;
        for (const Action& action : parser_table.get_states()[id].get_actions()) {
            std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) {},
                    [&](const Action::ReduceParameters& reduce_action) {
                        if (reduce_action.lookahead.empty()) {
                            already_has_default_action = true;
                        }
                        complete_lookahead_case(reduce_action.lookahead, unit_name, mappings, output);
                        output << "\n";
                        output << sfmt::Indentation{1};
                        if (!reduce_action.to_reduce.is_entry()) {
                            const size_t symbol_count = reduce_action.to_reduce.symbols.size(); 
                            const std::string production_name = reduce_action.to_reduce.name;
                            output << "this->reduce_stack(" << unit_name << "NonterminalType::" << upper_case_str(production_name)
                                   << ", " << symbol_count << ");\n"
                                   << "this->ast_builder.reduce_" << production_name << "(" << symbol_count << ");\n"
                                   << "break;\n";
                        } else {
                            output << "this->pop_many(" << reduce_action.to_reduce.symbols.size() << ");\n"
                                   << "return;\n";
                        }
                        output << sfmt::Indentation{-1};
                    },
                    [&](const Action::ShiftParameters& shift_action) {
                        if (shift_action.lookahead.empty()) {
                            already_has_default_action = true;
                        }
                        complete_lookahead_case(shift_action.lookahead, unit_name, mappings, output);
                        output << "\n    this->shift(" << shift_action.next_state << ");\n"
                                  "    break;\n"; 
                    }
                },
                action.parameters
            );
        }
        if (!already_has_default_action) {
            output << "default:\n"
                      "    this->call_error_handler(" << create_state_error_message(parser_table.get_states()[id]) << ", " 
                   << expected_tokens_to_string(parser_table.get_states()[id], unit_name) << ");\n";
            output << "    break;\n";
        }
        output << sfmt::Indentation{-1};
        output << "}\n"
                  "break;\n";

        output << sfmt::Indentation{-1};
    }
    output << sfmt::Indentation{-4};
}

void complete_lookahead_switch(const input::PalexConfig& config, std::ostream& output) {
    if (config.lookahead <= 1) {
        output << "this->lookahead.type";
    } else {
        output << "this->get_lookahead_id()";
    }
}

void complete_lookahead_case(
    const parser_generator::shift_reduce_parsers::Lookahead_t& lookahead, 
    const std::string& unit_name,
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t>& mappings,
    std::ostream& output
) {
    if (lookahead.empty()) {
        output << "default:";
    } else if (lookahead.size() == 1) {
        output << "case "<< unit_name << "Token::TokenType::" << lookahead[0].identifier << ":";
    } else {
        output << "case " << mappings.at(lookahead) << ":"; 
    }
}

void complete_lookahead_printer(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{1};
    if (config.lookahead < 2) {
        output << "error_message_stream << this->lookahead.type;";
    } else {
        output << "for (auto iter = this->lookahead.begin(); iter != this->lookahead.end(); iter++) {\n"
               << "    if (iter != this->lookahead.begin()) {\n"
               << "        error_message_stream << \", \";\n"
               << "    }\n"
               << "    error_message_stream << iter->type;\n"
               << "}";
    }
    output << sfmt::Indentation{-1};
}

void complete_position_printer(const input::PalexConfig& config, std::ostream& output) {
    if (config.lookahead < 2) {
        output << "this->lookahead.position";
    } else {
        output << "this->lookahead.front().position";
    }
}

std::string upper_case_str(const std::string& to_convert) {
    std::string result(to_convert.size(), '\0');
    std::transform(
        to_convert.begin(), 
        to_convert.end(), 
        result.begin(), 
        [](const char c) -> char { return (char)std::toupper(c); }
    );
    return result;
}
std::set<std::string> collect_nonterminal_names(const std::vector<parser_generator::Production>& productions) {
    std::set<std::string> nonterminals;
    for (const parser_generator::Production& production : productions) {
        if (!production.is_entry()) {
            nonterminals.insert(production.name);
        }
    }
    return nonterminals;
}

std::set<parser_generator::shift_reduce_parsers::Lookahead_t> collect_all_lookaheads(const parser_generator::shift_reduce_parsers::ParserTable& parser_table) {
    using namespace parser_generator::shift_reduce_parsers;

    std::set<Lookahead_t> lookaheads;
    for (const ParserState& state : parser_table.get_states()) {
        for (const Action& action : state.get_actions()) {
            std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) {},
                    [&](const Action::ReduceParameters& reduce_action) {
                        lookaheads.insert(reduce_action.lookahead);
                    },
                    [&](const Action::ShiftParameters& shift_action) {
                        lookaheads.insert(shift_action.lookahead);
                    }
                },
                action.parameters
            );
        }
    }
    return lookaheads;
}

std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> create_lookahead_mappings(
    const parser_generator::shift_reduce_parsers::ParserTable& parser_table
) {
    std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> lookahead_mapping;
    size_t current_id = 0;
    for (const parser_generator::shift_reduce_parsers::Lookahead_t& lookahead : collect_all_lookaheads(parser_table)) {
        lookahead_mapping[lookahead] = current_id++; 
    }
    return lookahead_mapping;
}

std::string create_state_error_message(const parser_generator::shift_reduce_parsers::ParserState& state) {
    using namespace parser_generator::shift_reduce_parsers;

    std::stringstream error_message_stream{};
    error_message_stream << "\"Received '\" + lookahead_to_string(this->lookahead) + \"', but expected one of the following: ";
    bool first_lookahead = true;
    for (const Action& action : state.get_actions()) {
        std::visit(
            Visitor{
                [](const Action::GotoParameters& goto_action) {},
                [&](const Action::ReduceParameters& reduce_action) {
                    if (!first_lookahead) {
                        error_message_stream << ", ";
                    }
                    first_lookahead = false;
                    error_message_stream << reduce_action.lookahead;
                },
                [&](const Action::ShiftParameters& shift_action) {
                    if (!first_lookahead) {
                        error_message_stream << ", ";
                    }
                    first_lookahead = false;
                    error_message_stream << shift_action.lookahead;
                }
            },
            action.parameters
        );
    }
    error_message_stream << ".\"";
    return error_message_stream.str();
}

std::string expected_tokens_to_string(const parser_generator::shift_reduce_parsers::ParserState& state, const std::string& unit_name) {
    using namespace parser_generator::shift_reduce_parsers;
    const auto output_lookahead_as_array = [&](const Lookahead_t& to_output, std::ostream& output) {
        output << "{";
        for (size_t i = 0; i < to_output.size(); i++) {
            if (i != 0) {
                output << ", ";
            }
            output << unit_name << "Token::TokenType::" << to_output[i].identifier;
        }
        output << "}";
    };

    std::stringstream expected_tokens{};
    expected_tokens << "{";
    bool first_lookahead = true;
    for (const Action& action : state.get_actions()) {
        std::visit(
            Visitor{
                [](const Action::GotoParameters& goto_action) {},
                [&](const Action::ReduceParameters& reduce_action) {
                    if (!first_lookahead) {
                        expected_tokens << ", ";
                    }
                    output_lookahead_as_array(reduce_action.lookahead, expected_tokens);
                    first_lookahead = false;
                },
                [&](const Action::ShiftParameters& shift_action) {
                    if (!first_lookahead) {
                        expected_tokens << ", ";
                    }
                    output_lookahead_as_array(shift_action.lookahead, expected_tokens);
                    first_lookahead = false;
                }
            },
            action.parameters
        );
    }
    expected_tokens << "}";
    return expected_tokens.str();
}

void complete_reduce_methods(const std::vector<parser_generator::Production>& productions, std::ostream& output) {
    output << sfmt::Indentation{3};
    for (const std::string& nonterminal_name : collect_nonterminal_names(productions)) {
        output << "virtual void reduce_" << nonterminal_name << "(const size_t child_count) = 0;\n";
    }
    output << sfmt::Indentation{-3};
}

void complete_lookahead_to_string(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{1};
    if (config.lookahead <= 1) {
        output << "string_repr << lookahead.type;";
        output << sfmt::Indentation{-1};
        return;
    }
    output << "for (auto iter = lookahead.begin(); iter != lookahead.end(); iter++) {\n";
    output << sfmt::Indentation{1};
    output << "if (iter != lookahead.begin()) {\n"
              "    string_repr << \", \";\n"
              "}\n"
              "string_repr << iter->type;\n";
    output << sfmt::Indentation{-1};
    output << "}";
    output << sfmt::Indentation{-1};
}

namespace parser_generator::shift_reduce_parsers::code_gen::cpp {
    bool generate_parser_files(
        const std::string& unit_name, 
        const std::vector<Production>& productions, 
        const ParserTable& parser_table, 
        const input::PalexConfig& config
    ) {
        try {
            generate_parser_header(unit_name, productions, config);
            generate_parser_source(unit_name, productions, parser_table, config);
            generate_ast_builder_header(unit_name, productions, config);
        } catch (const std::exception& err) {
            std::cerr << "Parser code generation failed: " << err.what() << std::endl;
            return false;
        }
        return true;
    }

    void generate_parser_header(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const input::PalexConfig& config
    ) {
        using namespace std::placeholders;

        const std::map<std::string_view, templates::TemplateCompleter_t> completers = { 
            {"LOOKAHEAD_INCLUDES", templates::conditional_completer(config.lookahead > 1, "#include <deque>")},
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
            {"LOOKAHEAD_COUNT", templates::constant_completer(std::to_string(config.lookahead))},
            {"NONTERMINAL_TYPES", std::bind(complete_nonterminal_enum, productions, _1)},
            {
                "LOOKAHEAD_TYPE",
                templates::choice_completer(config.lookahead > 1, "std::deque<" + unit_name + "Token>", unit_name + "Token")
            },
            {"LOOKAHEAD_FUNCTION", std::bind(complete_lookahead_function_declaration, config, _1)}
        };
        const std::string parser_header_path = config.output_path + "/" + unit_name + "Parser.h";
        std::cout << "Generating file " << parser_header_path << "..." << std::endl;
        templates::write_template_to_file(cpp_parser_header, parser_header_path, completers);
    }

    void generate_parser_source(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const ParserTable& parser_table,
        const input::PalexConfig& config
    ) {
        using namespace std::placeholders;

        const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
            {"PARSER_TABLE", std::bind(complete_parser_table, parser_table, unit_name, config, _1)},
            {"INIT_LOOKAHEAD", std::bind(complete_init_lookahead, config, _1)},
            {"SHIFT_FUNCTION", std::bind(complete_shift_function, config, _1)},
            {"GOTO_STATES", std::bind(complete_goto_states, parser_table, unit_name, _1)},
            {"REFILL_QUEUE", templates::conditional_completer(config.lookahead > 1, "this->init_lookahead();")},
            {"LOOKAHEAD_FUNCTION", std::bind(complete_lookahead_function, parser_table, unit_name, config, _1)},
            {
                "LOOKAHEAD_TYPE",
                templates::choice_completer(config.lookahead > 1, "std::deque<" + unit_name + "Token>", unit_name + "Token")
            },
            {"ERROR_REPORT_BEGIN", templates::choice_completer(config.lookahead > 1, "lookahead.front().begin", "lookahead.begin")},
            {"ERROR_REPORT_END", templates::choice_completer(config.lookahead > 1, "lookahead.front().end", "lookahead.end")},
            {
                "LOOKAHEAD_TO_STRING_FUNCTION", 
                std::bind(complete_lookahead_to_string, config, _1)
            },
            {
                "LOOKAHEAD_TYPE_OUTSIDE_NAMESPACE", 
                templates::choice_completer(
                    config.lookahead > 1, 
                    "std::deque<" + config.module_name + "::" +  unit_name + "Token>", 
                    config.module_name + "::" + unit_name + "Token"
                )
            },
            {"LOOKAHEAD_INCLUDES", templates::conditional_completer(config.lookahead > 1, "#include <map>")}
        };
        const std::string parser_source_path = config.output_path + "/" + unit_name + "Parser.cpp";
        std::cout << "Generating file " << parser_source_path << "..." << std::endl;
        templates::write_template_to_file(cpp_parser_source, parser_source_path, completers);
    }      

    void generate_ast_builder_header(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const input::PalexConfig& config
    ) {
        using namespace std::placeholders;

        const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
            {"REDUCE_METHODS", std::bind(complete_reduce_methods, productions, _1)}
        };
        const std::string ast_builder_header_path = config.output_path + "/" + unit_name + "ASTBuilderBase.h";
        std::cout << "Generating file " << ast_builder_header_path << "..." << std::endl;
        templates::write_template_to_file(cpp_ast_builder_header, ast_builder_header_path, completers);
    }      
}