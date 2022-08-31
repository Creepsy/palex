#include "RegexParser.h"

#include <algorithm>

#include "util/palex_except.h"

#include "character_classes.h"

using namespace regex;



const std::vector<std::string> regex::RegexParser::CHAR_TYPE_NAMES = {
    "SQUARE_OPEN",
    "SQUARE_CLOSE",
    "NEGATION",
    "ESCAPE",
    "MINUS",
    "DOT",
    "BRACKET_OPEN",
    "BRACKET_CLOSE",
    "PLUS",
    "STAR",
    "CURLY_OPEN",
    "CURLY_CLOSE",
    "COMMA",
    "ALTERNATION",
    "OPTIONAL",
    "CHARACTER"
};



// helper functions

bool is_single_char(const std::vector<CharRange>& to_check);
char32_t get_char(const std::vector<CharRange>& single_char);

bool is_single_char(const std::vector<CharRange>& to_check) {
    return to_check.size() == 1 && to_check.front().is_single_char();
}

char32_t get_char(const std::vector<CharRange>& single_char) {
    return single_char.front().start;
}



regex::RegexParser::RegexParser(const std::u32string& input) : input(input), curr_pos(0) {
}



// public

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex() {
    if(this->end()) return nullptr; //we dont have to process empty input

    return this->parse_regex_branch(); //TODO: maybe put assert for end() here
}



// private

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex_branch() {
    std::unique_ptr<RegexBase> sequence = this->parse_regex_sequence();

    if(this->accept(CharType::ALTERNATION)) {
        std::unique_ptr<RegexBranch> branch = std::make_unique<RegexBranch>();
        branch->add_possibility(std::move(sequence));

        while(this->accept(CharType::ALTERNATION)) {
            this->consume();
            branch->add_possibility(this->parse_regex_sequence());
        }

        return branch;
    } else {
        return sequence;
    }
}

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex_sequence() {
    const auto END_OF_SEQUENCE = [this]() -> bool { //TODO: better way to do this?
        return this->end() || this->accept(CharType::BRACKET_CLOSE) || this->accept(CharType::ALTERNATION);
    };

    std::unique_ptr<RegexBase> quantifier = this->parse_regex_quantifier();

    if(!END_OF_SEQUENCE()) {
        std::unique_ptr<RegexSequence> sequence = std::make_unique<RegexSequence>();
        sequence->append_element(std::move(quantifier));

        while(!END_OF_SEQUENCE()) {
            sequence->append_element(this->parse_regex_quantifier());
        }

        return sequence;
    }

    return quantifier;
}

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex_quantifier() {
    const auto IS_DIGIT = [](const char32_t to_check) -> bool { return to_check < 0xff && std::isdigit(to_check); };
    std::unique_ptr<RegexBase> operand = nullptr;

    if(this->accept(CharType::BRACKET_OPEN)) {
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
    } else if(this->accept(CharType::CURLY_OPEN)) {
        this->consume();
        std::u32string min_str = this->parse_matching_chars(IS_DIGIT);
        min = std::stoul(std::string(min_str.begin(), min_str.end()));
        max = min;

        if(this->accept(CharType::COMMA)) {
            this->consume();

            if(IS_DIGIT(this->get_curr())) {
                std::u32string max_str = this->parse_matching_chars(IS_DIGIT);
                max = std::stoul(std::string(max_str.begin(), max_str.end()));
            } else {
                max = RegexQuantifier::INFINITE;
            }
        }

        this->consume(CharType::CURLY_CLOSE);
    } else {
        return operand;
    }

    return std::make_unique<RegexQuantifier>(std::move(operand), min, max);
}

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex_charset() {
    if(this->accept(CharType::SQUARE_OPEN)) {
        this->consume();

        std::unique_ptr<RegexCharSet> char_set = std::make_unique<RegexCharSet>(this->accept(CharType::NEGATION));
        if(this->accept(CharType::NEGATION)) this->consume();

        while(!this->accept(CharType::SQUARE_CLOSE) && !this->end()) {
            for(const CharRange& range : this->parse_char_range()) {
                char_set->add_char_range(range);
            }
        }

        if(this->end()) this->throw_parsing_err(CharType::SQUARE_CLOSE);
        this->consume();

        return char_set;
    }

    std::unique_ptr<RegexCharSet> single_char = std::make_unique<RegexCharSet>(false);
    for(const CharRange& range : this->parse_char()) {
        single_char->add_char_range(range);
    }

    return single_char;
}

std::unique_ptr<RegexBase> regex::RegexParser::parse_regex_group() {
    this->consume(CharType::BRACKET_OPEN);

    std::unique_ptr<RegexBase> group = this->parse_regex_branch();

    this->consume(CharType::BRACKET_CLOSE);

    return group;
}

std::vector<CharRange> regex::RegexParser::parse_char(const bool inside_set) {
    if(this->accept(CharType::ESCAPE)) return this->parse_escaped_char();

    if(this->accept(CharType::DOT) && !inside_set) {
        this->consume();

        return character_classes::DOT_CLASS;
    }

    //TODO: are those even needed?
    switch(this->get_curr_type()) {
        case CharType::PLUS:
        case CharType::STAR:
        case CharType::OPTIONAL:
        case CharType::NEGATION:
        case CharType::SQUARE_OPEN:
        case CharType::BRACKET_OPEN:
        case CharType::BRACKET_CLOSE:
        case CharType::ALTERNATION:
            if(inside_set) break;
        case CharType::MINUS:
        case CharType::SQUARE_CLOSE:
            this->throw_parsing_err("This special character needs to be escaped in this context!");
            break;
    }

    char32_t curr = this->get_curr();
    this->consume();

    return std::vector<CharRange>{CharRange{curr}};
    
}

std::vector<CharRange> regex::RegexParser::parse_escaped_char() {
    if(!this->accept(CharType::ESCAPE)) return {};
    this->consume();

    const char32_t escaped = this->get_curr();
    this->consume();


    switch(escaped) {
        case 'c':
            {
                this->expect(CharType::CHARACTER);
                const char32_t curr = this->get_curr();
                this->consume();

                if(curr > 'Z' || curr < 'A') this->throw_parsing_err("Parameter of \\c has to be part of the upper-case alphabet!");
                
                return std::vector<CharRange>{CharRange{curr - 'A' + 1}};
            }
        case 'u':
            return std::vector<CharRange>{CharRange{this->parse_unicode_value()}};
        case 't':
            return std::vector<CharRange>{CharRange{'\t'}};
        case 'n':
            return std::vector<CharRange>{CharRange{'\n'}};
        case 'v':
            return std::vector<CharRange>{CharRange{'\v'}};
        case 'f':
            return std::vector<CharRange>{CharRange{'\f'}};
        case 'r':
            return std::vector<CharRange>{CharRange{'\r'}};
        case '0':
            return std::vector<CharRange>{CharRange{'\0'}};
        case 'w':
            return character_classes::WORD_CLASS;
        case 'W':
            return character_classes::NON_WORD_CLASS;
        case 'd':
            return character_classes::DIGIT_CLASS;
        case 'D':
            return character_classes::NON_DIGIT_CLASS;
        case 's':
            return character_classes::WHITESPACE_CLASS;
        case 'S':
            return character_classes::NON_WHITESPACE_CLASS;
        default:
            return std::vector<CharRange>{CharRange{escaped}};
    }
}

std::vector<CharRange> regex::RegexParser::parse_char_range() {
    std::vector<CharRange> first = this->parse_char(true);
    if(!is_single_char(first) || !this->accept(CharType::MINUS)) return first;
    this->consume();

    std::vector<CharRange> second = this->parse_char(true);
    if(!is_single_char(second)) this->throw_parsing_err("Expected single char as end of range!");

    return std::vector<CharRange>{CharRange{get_char(first), get_char(second)}};
}

char32_t regex::RegexParser::parse_unicode_value() {
    std::size_t unicode_value_length;

    if(this->accept(CharType::CURLY_OPEN)) {
        this->consume();
        std::size_t end = this->input.find_first_of('}', this->curr_pos);
        if(end == std::u32string::npos) this->throw_parsing_err(CharType::CURLY_CLOSE);
    
        unicode_value_length = end - this->curr_pos;
    } else {
        unicode_value_length = 4;
    }

    if(this->input.length() < this->curr_pos + unicode_value_length || unicode_value_length == 0) 
        this->throw_parsing_err("Tried to read past the end of the regex!");

    //TODO change this to use function parse_matching_chars
    std::u32string hex_string = this->input.substr(this->curr_pos, unicode_value_length);
    if(!std::all_of(
        hex_string.begin(),
        hex_string.end(),
        [](const char32_t to_check) -> bool { return to_check < 0xff && std::isxdigit(to_check); })
    ) {
        this->throw_parsing_err("Invalid hexadecimal value!");
    }

    this->curr_pos += unicode_value_length;
    if(this->accept(CharType::CURLY_CLOSE)) this->consume();

    return std::stoul(std::string(hex_string.begin(), hex_string.end()), nullptr, 16);
}

regex::RegexParser::CharType regex::RegexParser::get_curr_type() {
    switch(this->get_curr()) {
        case '[':
            return CharType::SQUARE_OPEN;
        case ']':
            return CharType::SQUARE_CLOSE;
        case '^':
            return CharType::NEGATION;
        case '\\':
            return CharType::ESCAPE;
        case '-':
            return CharType::MINUS;
        case '.':
            return CharType::DOT;
        case '(':
            return CharType::BRACKET_OPEN;
        case ')':
            return CharType::BRACKET_CLOSE;
        case '+':
            return CharType::PLUS;
        case '*':
            return CharType::STAR;
        case '{':
            return CharType::CURLY_OPEN;
        case '}':
            return CharType::CURLY_CLOSE;
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

std::u32string regex::RegexParser::parse_matching_chars(bool(*predicate)(const char32_t)) {
    size_t length = std::find_if_not(this->input.begin() + this->curr_pos, this->input.end(), predicate) - (this->input.begin() + this->curr_pos);
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

void regex::RegexParser::consume() {
    this->curr_pos++;
}

void regex::RegexParser::consume(const CharType type) {
    this->expect(type);
    this->consume();
}

bool regex::RegexParser::end() {
    return this->curr_pos >= this->input.length();
}

void regex::RegexParser::throw_parsing_err(const CharType expected) {
    this->throw_parsing_err("Unexpected character! Expected character of type " + RegexParser::CHAR_TYPE_NAMES[(size_t)expected] + "!");
}

void regex::RegexParser::throw_parsing_err(const std::string& message) {
    throw ParserError("Regex '" + std::string(this->input.begin(), this->input.end()) + "', Pos " + std::to_string(this->curr_pos) + ": " + message);
}