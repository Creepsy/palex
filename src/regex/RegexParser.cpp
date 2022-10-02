#include "RegexParser.h"

#include <algorithm>
#include <cassert>

#include "util/palex_except.h"
#include "util/unicode.h"

#include "character_classes.h"

// static variables

const std::vector<std::string> regex::RegexParser::CHAR_TYPE_NAMES = {
    "BRACKET_OPEN",
    "BRACKET_CLOSE",
    "NEGATION",
    "ESCAPE",
    "MINUS",
    "DOT",
    "PARENTHESIS_OPEN",
    "PARENTHESIS_CLOSE",
    "PLUS",
    "STAR",
    "BRACE_OPEN",
    "BRACE_CLOSE",
    "COMMA",
    "ALTERNATION",
    "OPTIONAL",
    "CHARACTER"
};



regex::RegexParser::RegexParser(const std::u32string& input) : input(input), curr_pos(0) {
}

// public

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex() {
    std::unique_ptr<RegexBase> parsed_regex = this->parse_regex_branch();
    if(this->accept(CharType::PARENTHESIS_CLOSE)) this->throw_parsing_err("The special character ')' has to be escaped in this context! Use \\).");

    assert(("RegexParser terminated before end! Please create an issue on github containing the used input!", this->end()));
    assert(("Regex is null! Please create an issue on github containing the used input!", parsed_regex));

    return parsed_regex;
}

// private

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex_branch() {
    std::unique_ptr<RegexBase> sequence = this->parse_regex_sequence();

    if(this->accept(CharType::ALTERNATION)) {
        std::unique_ptr<RegexAlternation> branch = std::make_unique<RegexAlternation>();
        branch->add_branch(std::move(sequence));

        while(this->accept(CharType::ALTERNATION)) {
            this->consume();
            branch->add_branch(this->parse_regex_sequence());
        }

        return branch;
    }

    return sequence;
}

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex_sequence() {
    std::vector<std::unique_ptr<RegexBase>> sequence_content = this->parse_until<std::unique_ptr<RegexBase>>(
        [](const char32_t c) -> bool {
            return c == ')' || c == '|';
        },
        &RegexParser::parse_regex_quantifier
    );

    if(sequence_content.empty()) this->throw_parsing_err("Empty sequences are not allowed, as they might lead to infinite matches!");
    if(sequence_content.size() == 1) return std::move(sequence_content.front());

    std::unique_ptr<RegexSequence> sequence = std::make_unique<RegexSequence>();

    for(std::unique_ptr<RegexBase>& element : sequence_content) {
        sequence->append_element(std::move(element));
    }

    return sequence;
}

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex_quantifier() {
    const auto IS_DIGIT = [](const char32_t to_check) -> bool { return to_check < 0xff && std::isdigit(to_check); };
    
    std::unique_ptr<RegexBase> operand = nullptr;

    if(this->accept(CharType::PARENTHESIS_OPEN)) {
        operand = this->parse_regex_group();
    } else {
        operand = this->parse_regex_charset();
    }

    size_t min, max;

    if(this->accept(CharType::PLUS)) {
        this->consume();
        min = 1;
        max = RegexQuantifier::INFINITE;
    } else if(this->accept(CharType::OPTIONAL)) {
        this->consume();
        min = 0;
        max = 1;
    } else if(this->accept(CharType::STAR)) {
        this->consume();
        min = 0;
        max = RegexQuantifier::INFINITE;
    } else if(this->accept(CharType::BRACE_OPEN)) {
        this->consume();

        std::u32string min_str = this->parse_matching_string(IS_DIGIT);
        if(min_str.empty()) this->throw_parsing_err("Expected a number!");
        min = std::stoul(unicode::to_utf8(min_str));
        max = min;

        if(this->accept(CharType::COMMA)) {
            this->consume();

            if(IS_DIGIT(this->get_curr())) {
                std::u32string max_str = this->parse_matching_string(IS_DIGIT);
                
                assert(("Number string empty! Please create an issue on github containing the used input!", !max_str.empty()));

                max = std::stoul(unicode::to_utf8(max_str));
            } else {
                max = RegexQuantifier::INFINITE;
            }
        }

        this->consume(CharType::BRACE_CLOSE);
    } else {
        return operand;
    }

    return std::make_unique<RegexQuantifier>(std::move(operand), min, max);
}

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex_charset() {
    if(this->accept(CharType::BRACKET_OPEN)) {
        this->consume();

        std::unique_ptr<RegexCharSet> char_set = std::make_unique<RegexCharSet>(this->accept(CharType::NEGATION));
        if(this->accept(CharType::NEGATION)) this->consume();

        const std::vector<MultiRangeCharacter> set_contents = this->parse_until<MultiRangeCharacter>(
            [](const char32_t c) -> bool {
                return c == ']';
            },
            &RegexParser::parse_char
        );
        this->consume(CharType::BRACKET_CLOSE);

        process_charset_contents(set_contents, *char_set);

        return char_set;
    }

    std::unique_ptr<RegexCharSet> regex_char = std::make_unique<RegexCharSet>(false);

    for(const CharRange& range : this->parse_char().first) {
        regex_char->insert_char_range(range);
    }

    return regex_char;
}

std::unique_ptr<regex::RegexBase> regex::RegexParser::parse_regex_group() {
    this->consume(CharType::PARENTHESIS_OPEN);

    std::unique_ptr<RegexBase> group = this->parse_regex_branch();

    this->consume(CharType::PARENTHESIS_CLOSE);

    return group;
}

regex::RegexParser::MultiRangeCharacter regex::RegexParser::parse_char() {
    if(this->accept(CharType::ESCAPE)) return this->parse_escaped_char();

    if(this->accept(CharType::DOT)) {
        this->consume();

        return {character_classes::DOT_CLASS, CharType::DOT};
    }

    const CharType returned_type = this->get_curr_type();
    
    return {std::vector<CharRange>{CharRange{this->consume()}}, returned_type};
    
}

regex::RegexParser::MultiRangeCharacter regex::RegexParser::parse_escaped_char() {
    if(!this->accept(CharType::ESCAPE)) return {};
    this->consume();

    const char32_t escaped = this->consume();

    switch(escaped) {
        case 'c':
            {
                const char32_t curr = this->consume(CharType::CHARACTER);
                if(curr > 'Z' || curr < 'A') this->throw_parsing_err("Parameter of \\c has to be part of the upper-case alphabet!");
                
                return {std::vector<CharRange>{CharRange{curr - 'A' + 1}}, CharType::CHARACTER};
            }
        case 'u':
            return {std::vector<CharRange>{CharRange{this->parse_unicode_value()}}, CharType::CHARACTER};
        case 't':
            return {std::vector<CharRange>{CharRange{'\t'}}, CharType::CHARACTER};
        case 'n':
            return {std::vector<CharRange>{CharRange{'\n'}}, CharType::CHARACTER};
        case 'v':
            return {std::vector<CharRange>{CharRange{'\v'}}, CharType::CHARACTER};
        case 'f':
            return {std::vector<CharRange>{CharRange{'\f'}}, CharType::CHARACTER};
        case 'r':
            return {std::vector<CharRange>{CharRange{'\r'}}, CharType::CHARACTER};
        case '0':
            return {std::vector<CharRange>{CharRange{'\0'}}, CharType::CHARACTER};
        case 'w':
            return {character_classes::WORD_CLASS, CharType::CHARACTER_CLASS};
        case 'W':
            return {character_classes::NON_WORD_CLASS, CharType::CHARACTER_CLASS};
        case 'd':
            return {character_classes::DIGIT_CLASS, CharType::CHARACTER_CLASS};
        case 'D':
            return {character_classes::NON_DIGIT_CLASS, CharType::CHARACTER_CLASS};
        case 's':
            return {character_classes::WHITESPACE_CLASS, CharType::CHARACTER_CLASS};
        case 'S':
            return {character_classes::NON_WHITESPACE_CLASS, CharType::CHARACTER_CLASS};
        default:
            return {std::vector<CharRange>{CharRange{escaped}}, CharType::CHARACTER};
    }
}

void regex::RegexParser::process_charset_contents(const std::vector<MultiRangeCharacter>& set_contents, RegexCharSet& target) {
    const auto IS_SINGLE_CHAR = [](const MultiRangeCharacter& to_check) -> bool {
        return to_check.second == CharType::DOT || (to_check.first.size() == 1 && to_check.first.front().is_single_char());
    };
    const auto GET_SINGLE_CHAR = [](const MultiRangeCharacter& to_unwrap) -> char32_t {
        return (to_unwrap.second == CharType::DOT) ? '.' : to_unwrap.first.front().start;
    };

    for(size_t i = 0; i < set_contents.size(); i++) {
        switch(set_contents[i].second) {
            case CharType::CHARACTER_CLASS:
                for(const CharRange range : set_contents[i].first) target.insert_char_range(range);
                break;
            default:
                assert(("Expected native character! Please create an issue on github containing the used input!", IS_SINGLE_CHAR(set_contents[i])));
                char32_t start = GET_SINGLE_CHAR(set_contents[i]);

                if(i < set_contents.size() - 2 && set_contents[i + 1].second == CharType::MINUS && IS_SINGLE_CHAR(set_contents[i + 2])) {
                    target.insert_char_range(CharRange{start, GET_SINGLE_CHAR(set_contents[i + 2])});
                    i += 2;
                } else {
                    target.insert_char_range(CharRange{start});
                }
                break;
        }
    }
}

char32_t regex::RegexParser::parse_unicode_value() {
    const auto IS_HEX_DIGIT = [](const char32_t to_check) -> bool { return to_check < 0xff && std::isxdigit(to_check); };
    
    std::u32string unicode_value;

    if(this->accept(CharType::BRACE_OPEN)) {
        this->consume();
        unicode_value = this->parse_matching_string(IS_HEX_DIGIT);
        this->consume(CharType::BRACE_CLOSE);
    } else {
        unicode_value = this->parse_matching_string(IS_HEX_DIGIT, 4);
        if(unicode_value.length() != 4)
            this->throw_parsing_err("Expected 4 hexadecimal characters, found " + std::to_string(unicode_value.length()) + "!");
    }

    if(unicode_value.empty()) this->throw_parsing_err("Expected a hexadecimal value!");

    char32_t unicode_char = (char32_t)std::stoul(unicode::to_utf8(unicode_value), nullptr, 16);
    if(unicode_char > unicode::LAST_UNICODE_CHAR) this->throw_parsing_err("Invalid unicode value with code " + std::to_string(unicode_char));

    return unicode_char;
}

regex::RegexParser::CharType regex::RegexParser::get_curr_type() {
    switch(this->get_curr()) {
        case '[':
            return CharType::BRACKET_OPEN;
        case ']':
            return CharType::BRACKET_CLOSE;
        case '^':
            return CharType::NEGATION;
        case '\\':
            return CharType::ESCAPE;
        case '-':
            return CharType::MINUS;
        case '.':
            return CharType::DOT;
        case '(':
            return CharType::PARENTHESIS_OPEN;
        case ')':
            return CharType::PARENTHESIS_CLOSE;
        case '+':
            return CharType::PLUS;
        case '*':
            return CharType::STAR;
        case '{':
            return CharType::BRACE_OPEN;
        case '}':
            return CharType::BRACE_CLOSE;
        case ',':
            return CharType::COMMA;
        case '|':
            return CharType::ALTERNATION;
        case '?':
            return CharType::OPTIONAL;
        default:
            return CharType::CHARACTER;
    }
}

char32_t regex::RegexParser::get_curr() {
    if(this->end()) this->throw_parsing_err("Tried to read past the end of the regex!");
    return this->input[this->curr_pos];
}

std::u32string regex::RegexParser::parse_matching_string(Predicate_t predicate, const size_t max_count) {
    size_t length = std::find_if_not(this->input.begin() + this->curr_pos, this->input.end(), predicate) - (this->input.begin() + this->curr_pos);
    if(length > max_count) length = max_count;
    this->curr_pos += length;

    return this->input.substr(this->curr_pos - length, length);
}

void regex::RegexParser::expect(const CharType type) {
    if(!this->accept(type)) this->throw_parsing_err(type);
}

bool regex::RegexParser::accept(const CharType type) {
    if(this->end()) return false;

    return this->get_curr_type() == type;
}

char32_t regex::RegexParser::consume() {
    char32_t consumed = this->get_curr();
    this->curr_pos++;

    return consumed;
}

char32_t regex::RegexParser::consume(const CharType type) {
    this->expect(type);
    return this->consume();
}

bool regex::RegexParser::end() {
    return this->curr_pos >= this->input.length();
}

void regex::RegexParser::throw_parsing_err(const CharType expected) {
    this->throw_parsing_err("Unexpected character! Expected character of type " + RegexParser::CHAR_TYPE_NAMES.at((size_t)expected) + "!");
}

void regex::RegexParser::throw_parsing_err(const std::string& message) {
    throw palex_except::ParserError("Regex '" + std::string(this->input.begin(), this->input.end()) + "', Pos " + std::to_string(this->curr_pos) + ": " + message);
}