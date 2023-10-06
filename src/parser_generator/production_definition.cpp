#include "production_definition.h"

#include <cassert>

#include "util/Visitor.h"

namespace parser_generator {
    bool Production::is_entry() const {
        return this->name == ENTRY_PRODUCTION_NAME;
    }

    std::string Production::get_representation() const {
        return this->tag.empty() ? this->name : this->name + "_" + this->tag;
    }

    std::optional<std::string> Production::get_astbuilder_method_name() const {
        if (this->is_entry()) {
            return std::nullopt;
        }
        return std::visit(Visitor{
            [this](const ErrorResult_t& err) -> std::optional<std::string> {
                if (err.has_value()) {
                    return std::nullopt;
                }
                return "error_" + this->get_representation();
            },
            [this](const ReductionResult_t& reduction) -> std::optional<std::string> {
                return "reduce_" + this->get_representation();
            }
        }, this->result);
    }

    bool operator<(const Symbol& first, const Symbol& second) {
        if (first.type != second.type) return first.type < second.type;
        return first.identifier < second.identifier;
    }

    bool operator<(const Production& first, const Production& second) {
        if (first.name != second.name) {
            return first.name < second.name;
        }
        if (first.symbols != second.symbols) {
            return first.symbols < second.symbols;
        }
        return first.result < second.result;
    }

    bool operator==(const Symbol& first, const Symbol& second) {
        return first.type == second.type && first.identifier == second.identifier;
    }

    bool operator==(const Production& first, const Production& second) {
        return first.name == second.name && first.symbols == second.symbols && first.result == second.result;
    }

    bool operator!=(const Symbol& first, const Symbol& second) {
        return !(first == second);
    }

    bool operator!=(const Production& first, const Production& second) {
        return !(first == second);
    }

    std::ostream& operator<<(std::ostream& output, const Symbol& to_print) {
        if (to_print.type == Symbol::SymbolType::TERMINAL) {
            return output << to_print.identifier;
        }
        return output << to_print.identifier;
    }

    std::ostream& operator<<(std::ostream& output, const Production& to_print) {
        output << to_print.name;
        if (!to_print.tag.empty()) {
            output << "#" << to_print.tag;
        }
        output << " =";
        for (const Symbol& sym : to_print.symbols) {
            output << " " << sym;
        }
        std::visit(Visitor{
            [&](const ErrorResult_t& error_result) {
                output << " -> error";
                if (error_result.has_value()) {
                    output << " \"" << error_result.value() << '"';
                }
            },
            [](const auto& other) {}
        }, to_print.result);
        return output;
    }
}