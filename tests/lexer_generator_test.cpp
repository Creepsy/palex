#include <sstream>

#include "lexer_generator/LexerRuleLexer.h"
#include "lexer_generator/LexerRuleParser.h"
#include "lexer_generator/validation.h"

#include "util/palex_except.h"

#include "TestReport.h"
#include "test_utils.h"

bool test_rule_lexer();
bool test_rule_parser();
bool test_rule_parser_errors();

bool test_rule_validation();

int main() {
    tests::TestReport report;

    report.add_test("rule_lexer", test_rule_lexer);
    report.add_test("rule_parser", test_rule_parser);
    report.add_test("rule_parser_errors", test_rule_parser_errors);

    report.add_test("rule_validation", test_rule_validation);

    report.run();

    return report.report();
}

#include <iostream>

bool test_rule_lexer() {
    std::stringstream input("$ IDENT iDEnT _iden1 \"regex\" \"\\\"with quote\" = ; \"undefined\n\"\" ^<>6ust");

    lexer_generator::LexerRuleLexer lexer(input);

    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IGNORE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::EQUALS)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_LINE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::REGEX)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::UNDEFINED)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::ANGLE_PARENTHESIS_OPEN)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::ANGLE_PARENTHESIS_CLOSE)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::INTEGER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::IDENTIFER)
    TEST_TRUE(lexer.next_token().type == lexer_generator::Token::TokenType::END_OF_FILE)

    return true;
}

bool test_rule_parser() {
    std::stringstream input("$WSPACE=\"\\s+\"; _IDE_NT2 =     \"\\w+\";  ");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);
    
    const lexer_generator::TokenRegexRule rule_wspace = parser.parse_token_rule().value();
    TEST_TRUE(rule_wspace.ignore_token)
    TEST_TRUE(rule_wspace.name == U"WSPACE")
    TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(rule_wspace.regex_ast.get()));

    const lexer_generator::TokenRegexRule rule_ident = parser.parse_token_rule().value();
    TEST_FALSE(rule_ident.ignore_token)
    TEST_TRUE(rule_ident.name == U"_IDE_NT2")
    TEST_TRUE(dynamic_cast<regex::RegexQuantifier*>(rule_ident.regex_ast.get()));

    TEST_FALSE(parser.parse_token_rule().has_value())
    
    return true;
}

bool test_rule_parser_errors() {
    std::stringstream input("$WSPACE = IDENT = \"aaaaa\"\n T2 =");
    
    lexer_generator::LexerRuleLexer lexer(input);
    lexer_generator::LexerRuleParser parser(lexer);

    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)
    TEST_EXCEPT(parser.parse_token_rule(), palex_except::ParserError)

    TEST_FALSE(parser.parse_token_rule().has_value())
    
    return true;
}

bool test_rule_validation() {
    struct TestCase {
        std::string input;

        bool should_fail;
    };

    const std::vector<TestCase> TEST_CASES = {
        {"IDENTIFIER = \"a\"; INTEGER = \"a\"; _INT = \"a\"; _123WEIRD_IDENT = \"a\";", false},
        {"UNDEFINED = \"a\";", true},
        {"END_OF_FILE = \"a\";", true},
        {"DUP_IDENT = \"a\"; DUP_IDENT = \"a\";", true}
    };

    for(const TestCase& test : TEST_CASES) {
        std::stringstream input(test.input);
        lexer_generator::LexerRuleLexer lexer(input);
        lexer_generator::LexerRuleParser parser(lexer);
        
        std::vector<lexer_generator::TokenRegexRule> rules = parser.parse_all_rules();

        if(test.should_fail) {
            TEST_EXCEPT(lexer_generator::validate_rules(rules), palex_except::ValidationError);
        } else {
            lexer_generator::validate_rules(rules);
        }
    }

    return true;
}