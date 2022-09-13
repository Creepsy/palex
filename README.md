# PaLex
![GitHub CI](https://github.com/Creepsy/palex/actions/workflows/tests.yml/badge.svg)

PaLex is a parser and lexer generator written in C++. It allows the creation of lexers and parsers through rule files. The generated code supports ASCII and UTF-8 input. The generator currently supports the following languages: **TODO**

## Getting started / example
**TODO**

## Lexer generator

### Naming of the rule file
In order the be recognized by the parser, a lexer rule file has to have the file extension ".lrules". The file name specifies the name of the generated lexer.

### Token rule syntax
A token is defined through the token name followed by an equal sign and the corresponding regex. Every token definition ends with a semicolon. A few valid examples for rule definitions are:

```
INTEGER = "\d+";
INT = "int";
```

Token names should always be written with upper-case and underscores. Please note that some names are reserved by the generator itself (For further information see "Naming limitations"). Rules are allowed to use unicode characters, as long as the rule files is stored in UTF-8 format.
As the lexer isn't able to skip characters, all characters from the input stream have to be processed. However, there exists an option to ignore specific tokens (For further information see "Token ignore list").

### Naming limitations
Some token names are reserved by the generator itself, and therefore can't be used. These are: UNDEFINED, END_OF_FILE

### Regex limitations
Not the whole regex standard is supported by this generator. The following features are currently not supported:
- anchors
- backreferences
- lookaheads

### Token ignore list
This feature is intended to be used for seperation tokens like whitespace. By putting a token on the ignore list, tokens of this type are no longer returned by the lexer. To put a token on the ignore list, simply add a `$` character in front of the token rule definition:
```
$WSPACE = "\s+";
```

### Special tokens
When an invalid input sequence is recognized by the lexer, a token with the type of UNDEFINED is returned by the lexer. The invalid tokens get consumed which allows the user to continue with parsing tokens from the input stream. A token of the type END_OF_FILE is returned when the input stream contains no more characters to read.