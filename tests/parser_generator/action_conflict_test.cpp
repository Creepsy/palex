#include <vector>

#include "parser_generator/shift_reduce_parsers/parser_state.h"

#include "../test_utils.h"

struct TestCase {
    parser_generator::shift_reduce_parsers::Action first;
    parser_generator::shift_reduce_parsers::Action second;
    bool conflict;
};

int main() {
    using parser_generator::shift_reduce_parsers::Action;
    using parser_generator::Symbol;

    const parser_generator::Production prodA{"multiplication"};
    const parser_generator::Production prodB{"addition"};
    const std::vector<TestCase> TEST_CASES = {
        TestCase{
            Action{Action::GotoParameters{0, Symbol{Symbol::SymbolType::NONTERMINAL, "addition"}}},
            Action{Action::GotoParameters{0, Symbol{Symbol::SymbolType::NONTERMINAL, "multiplication"}}},
            false
        },
        TestCase{
            Action{Action::GotoParameters{0, Symbol{Symbol::SymbolType::NONTERMINAL, "addition"}}},
            Action{Action::GotoParameters{0, Symbol{Symbol::SymbolType::NONTERMINAL, "addition"}}},
            false
        },
        TestCase{ // this conflict shouldn't occur in practice
            Action{Action::GotoParameters{0, Symbol{Symbol::SymbolType::NONTERMINAL, "addition"}}},
            Action{Action::GotoParameters{1, Symbol{Symbol::SymbolType::NONTERMINAL, "addition"}}},
            true
        },
        TestCase{
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            false
        },
        TestCase{
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ShiftParameters{1, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "FLOAT"}}}},
            false
        },
        TestCase{ // this conflict shouldn't occur in practice
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ShiftParameters{1, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            true
        },
        TestCase{
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            false
        },
        TestCase{
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "FLOAT"}}}},
            false
        },
        TestCase{
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodB, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "FLOAT"}}}},
            false
        },
        TestCase{
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodB, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            true
        },
        TestCase{
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "FLOAT"}}}},
            false
        },
        TestCase{
            Action{Action::ShiftParameters{0, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            Action{Action::ReduceParameters{prodA, std::vector<Symbol>{Symbol{Symbol::SymbolType::TERMINAL, "INT"}}}},
            true
        }
    };
    for (const TestCase& test : TEST_CASES) {
        TEST_TRUE(Action::conflict(test.first, test.second) == test.conflict);
    }
    return 0;
}