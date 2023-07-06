#include "cpp_code_gen.h"

#include <iostream>
#include <stdexcept>
#include <map>
#include <string_view>
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

const std::string LOOKAHEAD_FUNCTION_COMPLETION = 
R"(size_t %MODULE_NAMESPACE%::%UNIT_NAME%Parser::get_lookahead_id() const {
    const static std::map<std::array<Token::TokenType, LOOKAHEAD>, size_t> lookahead_mappings = {
%LOOKAHEAD_MAPPINGS%    };

    std::array<Token::TokenType, LOOKAHEAD> lookahead_key;
    size_t i = 0;
    for (auto iter = this->lookahead.begin(); iter != this->lookahead.end(); iter++) {
        lookahead_key[i++] = iter->type;
    }
    const auto mapping = lookahead_mappings.find(lookahead_key);
    return (mapping == lookahead_mappings.end()) ? ERROR_STATE : mapping->second; 
})";

// helper functions
void complete_lookahead_type(const input::PalexConfig& config, std::ostream& output);
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
void complete_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, std::ostream& output);
void complete_goto_states(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, std::ostream& output);
void complete_parser_table(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const input::PalexConfig& config, std::ostream& output);
void complete_lookahead_switch(const input::PalexConfig& config, std::ostream& output);
void complete_lookahead_case(
    const parser_generator::shift_reduce_parsers::Lookahead_t& lookahead, 
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t>& mappings,
    std::ostream& output);
std::string upper_case_str(const std::string& to_convert);
std::set<std::string> collect_nonterminal_names(const std::vector<parser_generator::Production>& productions);
std::set<parser_generator::shift_reduce_parsers::Lookahead_t> collect_all_lookaheads(const parser_generator::shift_reduce_parsers::ParserTable& parser_table);
std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> create_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table);

void complete_lookahead_type(const input::PalexConfig& config, std::ostream& output) {
    if (config.lookahead <= 1) {
        output << "Token lookahead;";
    } else {
        output << "std::deque<Token> lookahead;";
    }
}

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
    output << "_START\n";
    output << sfmt::Indentation{-2};
}

void complete_init_lookahead(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{1};
    if (config.lookahead <= 1) {
        output << "this->lookahead = this->lexer.next_unignored_token();";
    } else {
        output << "for (size_t _ = 0; _ < LOOKAHEAD; _++) {\n"
                  "    this->lookahead.push_back(this->lexer.next_unignored_token());\n"
                  "}";
    }
    output << sfmt::Indentation{-1};
}

void complete_shift_function(const input::PalexConfig& config, std::ostream& output) {
    output << sfmt::Indentation{1};
    if (config.lookahead <= 1) {
        output << "std::unique_ptr<TokenAstNode> token_ast = std::make_unique<TokenAstNode>(this->lookahead);\n"
                  "this->lookahead = this->lexer.next_unignored_token();\n"
                  "this->parser_stack.push(std::make_pair(std::move(token_ast), next_state));";
    } else {
        output << "std::unique_ptr<TokenAstNode> token_ast = std::make_unique<TokenAstNode>(this->lookahead.front());\n"
                  "this->lookahead.pop_front();\n"
                  "this->lookahead.push_back(this->lexer.next_unignored_token());\n"
                  "this->parser_stack.push(std::make_pair(std::move(token_ast), next_state));";
    }
    output << sfmt::Indentation{-1};
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
        {"LOOKAHEAD_MAPPINGS", std::bind(complete_lookahead_mappings, parser_table, _1)}
    };
    templates::write_template_to_stream(LOOKAHEAD_FUNCTION_COMPLETION.c_str(), output, completers);
}

void complete_lookahead_mappings(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, std::ostream& output) {
    output << sfmt::Indentation{2};
    for (const auto& [lookahead, id] : create_lookahead_mappings(parser_table)) {
        output << "{{";
        for (size_t i = 0; i < lookahead.size(); i++) {
            if (i != 0) {
                output << ", ";
            }
            output << "Token::TokenType::" << lookahead[i].identifier;
        }
        output << "}, " << id << "},\n";
    }
    output << sfmt::Indentation{-2};
}

void complete_goto_states(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, std::ostream& output) {
    using namespace parser_generator::shift_reduce_parsers;

    output << sfmt::Indentation{2};
    for (size_t id = 0; id < parser_table.get_states().size(); id++) {
        output << "case " << id << ":\n";
        output << sfmt::Indentation{1};
        output << "switch (reduced_production) {\n";
        output << sfmt::Indentation{1};
        for (const Action& action : parser_table.get_states()[id].get_actions()) {
            std::visit(
                Visitor{
                    [&](const Action::GotoParameters& goto_action) {
                        output << "case NonterminalType::" << upper_case_str(goto_action.reduced_symbol.identifier) << ":\n"
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
    output << sfmt::Indentation{-2};
}

void complete_parser_table(const parser_generator::shift_reduce_parsers::ParserTable& parser_table, const input::PalexConfig& config, std::ostream& output) {
    using namespace parser_generator::shift_reduce_parsers;
   
    output << sfmt::Indentation{3};
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t> mappings = create_lookahead_mappings(parser_table);
    for (size_t id = 0; id < parser_table.get_states().size(); id++) {
        output << "case " << id << ":\n";
        output << sfmt::Indentation{1};
        output << "switch (";
        complete_lookahead_switch(config, output);
        output << ") {\n";
        output << sfmt::Indentation{1};
        for (const Action& action : parser_table.get_states()[id].get_actions()) {
            std::visit(
                Visitor{
                    [](const Action::GotoParameters& goto_action) {},
                    [&](const Action::ReduceParameters& reduce_action) {
                        complete_lookahead_case(reduce_action.lookahead, mappings, output);
                        output << "\n";
                        output << sfmt::Indentation{1};
                        const std::string production_name = reduce_action.to_reduce.is_entry() ? "_START" : upper_case_str(reduce_action.to_reduce.name);
                        output << "this->reduce_n(" << reduce_action.to_reduce.symbols.size() 
                               << ", NonterminalType::" << production_name << ");\n";
                        if (reduce_action.to_reduce.is_entry()) {
                            output << "{\n";
                            output << sfmt::Indentation{1};
                            output << "std::unique_ptr<AstNode> result = std::move(this->parser_stack.top().first);\n"
                                      "this->parser_stack.pop();\n"
                                      "return std::move(result);\n";
                            output << sfmt::Indentation{-1};
                            output << "}\n";
                        } else {
                            output << "break;\n";
                        }
                        output << sfmt::Indentation{-1};
                    },
                    [&](const Action::ShiftParameters& shift_action) {
                        complete_lookahead_case(shift_action.lookahead, mappings, output);
                        output << "\n    this->shift(" << shift_action.next_state << ");\n"
                                  "    break;\n"; 
                    }
                },
                action.parameters
            );
        }
        output << "default:\n"
                  "    throw std::runtime_error(\"Parser Error, TODO better error message\");\n";
        output << sfmt::Indentation{-1};
        output << "}\n"
                  "break;\n";

        output << sfmt::Indentation{-1};
    }
    output << sfmt::Indentation{-3};
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
    const std::map<parser_generator::shift_reduce_parsers::Lookahead_t, size_t>& mappings,
    std::ostream& output
) {
    if (lookahead.empty()) {
        output << "default:";
    } else if (lookahead.size() == 1) {
        output << "case Token::TokenType::" << lookahead[0].identifier << ":";
    } else {
        output << "case " << mappings.at(lookahead) << ":"; 
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

namespace parser_generator::shift_reduce_parsers::code_gen::cpp {
    bool generate_parser_files(
        const std::string& unit_name, 
        const std::vector<Production>& productions, 
        const ParserTable& parser_table, 
        const input::PalexConfig& config
    ) {
        try {
            generate_parser_header(unit_name, config);
            generate_parser_source(unit_name, productions, parser_table, config);
            if (config.generate_util) {
                generate_types_header(unit_name, productions, config);
                generate_types_source(unit_name, config);
            }
        } catch (const std::exception& err) {
            std::cerr << "Parser code generation failed: " << err.what() << std::endl;
            return false;
        }
        return true;
    }

    void generate_parser_header(
        const std::string& unit_name,
        const input::PalexConfig& config
    ) {
        using namespace std::placeholders;

        const std::map<std::string_view, templates::TemplateCompleter_t> completers = { 
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
            {"LOOKAHEAD_TYPE", std::bind(complete_lookahead_type, config, _1)},
            {"LOOKAHEAD_FUNCTION", std::bind(complete_lookahead_function_declaration, config, _1)}
        };
        const std::string parser_header_path = config.output_path + "/" + unit_name + "Parser.h";
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
            {"LOOKAHEAD_COUNT", templates::constant_completer(std::to_string(config.lookahead))},
            {"INIT_LOOKAHEAD", std::bind(complete_init_lookahead, config, _1)},
            {"GOTO_STATES", std::bind(complete_goto_states, parser_table, _1)},
            {"LOOKAHEAD_FUNCTION", std::bind(complete_lookahead_function, parser_table, unit_name, config, _1)},
            {"SHIFT_FUNCTION", std::bind(complete_shift_function, config, _1)},
            {"PARSER_TABLE", std::bind(complete_parser_table, parser_table, config, _1)}
        };
        const std::string parser_source_path = config.output_path + "/" + unit_name + "Parser.cpp";
        templates::write_template_to_file(cpp_parser_source, parser_source_path, completers);
    }            

    void generate_types_header(
        const std::string& unit_name,
        const std::vector<Production>& productions,
        const input::PalexConfig& config
    ) {
        using namespace std::placeholders;

        const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)},
            {"NONTERMINAL_ENUM", std::bind(complete_nonterminal_enum, productions, _1)}
        };
        const std::string types_header_path = config.output_path + "/" + unit_name + "Types.h";
        templates::write_template_to_file(cpp_types_header, types_header_path, completers);
    }

    void generate_types_source(
        const std::string& unit_name,
        const input::PalexConfig& config
    ) {
        const std::map<std::string_view, templates::TemplateCompleter_t> completers = {
            {"UNIT_NAME", templates::constant_completer(unit_name)},
            {"MODULE_NAMESPACE", templates::constant_completer(config.module_name)}
        };
        const std::string types_source_path = config.output_path + "/" + unit_name + "Types.cpp";
        templates::write_template_to_file(cpp_types_source, types_source_path, completers);
    }
}