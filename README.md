# Palex
![GitHub CI](https://github.com/Creepsy/palex/actions/workflows/tests.yml/badge.svg)

Palex is a parser and lexer generator written in C++. It allows the creation of lexers and parsers supporting UTF-8 input. The following languages are currently supported:

| Language | Lexer Generator | Parser Generator |
| :------: | :-------------: | :--------------: |
| C++      | Yes             | Yes              |

## Table of Contents
- [Palex](#palex)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Using Palex in your project](#using-palex-in-your-project)
  - [Contributing](#contributing)
  - [Lexer grammar](#lexer-grammar)
    - [Token rule syntax](#token-rule-syntax)
    - [Naming limitations](#naming-limitations)
    - [Regex limitations](#regex-limitations)
    - [Token ignore list](#token-ignore-list)
    - [Special tokens](#special-tokens)
    - [Token priority](#token-priority)
  - [Parser grammar](#parser-grammar)
    - [Production syntax](#production-syntax)
    - [Parser entry production](#parser-entry-production)
    - [Production tags](#production-tags)
  - [Command line arguments](#command-line-arguments)
    - [Options](#options)
    - [Flags](#flags)

## Getting Started
In order to clone the repository and build Palex run: 
```
git clone https://github.com/Creepsy/palex.git
cd palex
mkdir build && cd build
cmake .. && make
```

In addition you can also install Palex in the `/usr/local/bin` directory by running 
```
sudo make install
```
in your build folder.

In case you want to try out the example go to the example folder and build the project (note that the example project requires you to have palex installed on your system):
```
mkdir build && cd build
cmake .. && make
```

### Using Palex in your project
To start using Palex in your own projects you need to create a rule file first. Rule files should end with the file extension `.palex`. The name of the rule file also specifies the name of the generated lexer and parser. Consequently, a rule file named `My.palex` produces the lexer `MyLexer` and the parser `MyParser`.

Every rule file begins with the token definitions which are followed by the grammar for the parser. Here is a simple rule file for recognizing additions you can use for testing:
```
!WSPACE = "\s+";
INT = "\d+";
ADD = "+";

$S = addition;
addition = addition ADD number;
addition = number;
number = INT;
```
For more information about the structure of rule files, visit the corresponding sections for [lexer grammar](#lexer-grammar) and [parser grammar](#parser-grammar).
Simply put these rules in a rule file and run
```
palex YourRuleFile.palex -lang C++ -parser-type LALR --lexer --util --parser
```
Note that the lang and parser-type flags are **always** required. This command will create the lexer and parser files in the current working directory for C++. Available command line arguments and their default configurations are described in more detail [here](#command-line-arguments).

Running the command should result in a folder structure similar to this:
- project_folder
  - YourRuleFile.palex
  - **YourRuleFileLexer.h**
  - **YourRuleFileLexer.cpp**
  - **YourRuleFileParser.h**
  - **YourRuleFileParser.cpp**
  - **YourRuleFileASTBuilderBase.h**
  - **YourRuleFileASTBuilderBase.cpp**
  - **utf8.h**          
  - **utf8.cpp**   
  - **YourRuleFileToken.h**   
  - **YourRuleFileToken.cpp**  
  
You can continue from here by using the generated files in your project or by integrating the generator in your build process (An example for CMake can be found in the example folder).
Congrats! You just created your first project with Palex!

## Contributing
For information how you can contribute to this project visit the [contribute section](https://github.com/Creepsy/palex/blob/main/CONTRIBUTING.md) of this project.

## Lexer grammar
Every rule file starts with the token definitions for the lexer which are then followed by the grammar of the language. The following section will describe the former in more detail.
### Token rule syntax
A token is defined through the token name followed by an equal sign and the corresponding regex. Every token definition ends with a semicolon. A few valid examples for token definitions are:

```
INTEGER = "\d+";
INT = "int";
```

Token names are required to consist of uppercase letters and underscores only (upper snake-case). Please note that [some names are reserved](#naming-limitations) by the generator itself. Rules are allowed to use unicode characters, as long as the rule file is stored in UTF-8 format.
As the lexer isn't able to skip characters, all characters from the input stream have to be processed. However, there exists an option to [ignore specific tokens](#token-ignore-list).

### Naming limitations
Some token names are reserved by the generator itself, and therefore can't be used. These are: UNDEFINED, END_OF_FILE

### Regex limitations
This generator supports most of the commonly used regex features. Please note however, that the following (widely used) features are currently not supported:
- anchors
- backreferences
- lookaheads

If you want to extend the functionality of the regex parser, feel free to do so. For more information on how to make a contribution to the project visit the corresponding [section](#contributing).

### Token ignore list
This feature is intended to be used for seperation tokens like whitespace. By putting a token on the ignore list, tokens of this type are no longer returned by the lexer. To put a token on the ignore list, simply add a `!` character in front of the token rule definition:
```
!WSPACE = "\s+";
```
Ignored tokens are still being processed by the method `next_token()` while they are skipped by the method `next_unignored_token()`.

### Special tokens
When an invalid input sequence is recognized by the lexer, a token with the type of UNDEFINED is returned by the lexer. The invalid tokens get consumed which allows the user to continue with parsing tokens from the input stream. A token of the type END_OF_FILE is returned when the input stream contains no more characters to read.

### Token priority
The generator has an automatic priority system for tokens:

- The priority of an alternation is the smallest priority of its branches
- The priority of a character set is always 1
- The priority of a normal character is always 2
- The priority of a quantifier is the minimal matching count multiplied with the priority of the operand
- The priority of a sequence is sum of all priorities of its elements

Thereby a token with a higher priority is choosen over one with a lower priority.

Even though most conflicts can can already be resolved by the generator itself, some cases still require the user to resolve the conflict by hand:

```
ALPHA_NUM = "[a-z0-9]+";
NUM = "[0-9]+";
```

In this case both token rules have a priority of 1. Therefore, the generator can not decide which type a numeric string should have. To resolve this, the user can set the priority of a token manually by putting the priority level encased in `<>` on front of the token name:

```
ALPHA_NUM = "[a-z0-9]+";
<2>NUM = "[0-9]+";
```

Keep in mind, that **priority levels always have to be >= 0**. In case you want to set the priority of an ignored token, the **ignore tag `!` has to stand before the priority tag**. 

## Parser grammar
This section will explain the grammar format for parsers in more detail.
The grammar is quite similar to normal BNF-grammars. Therefore it is advised to make sure that one understands them first before continuing reading.

### Production syntax
A grammar consists out of a set of productions whereas a production is made out of terminals and nonterminals. The terminals are the tokens we defined earlier in the lexer section. The nonterminals are the results of other productions of our grammar.

Every production starts with the resulting nonterminal on the left, immediately followed by an equal sign. The right side is a sequence of terminals and nonterminals and every production is terminated with a semicolon. While the terminals have the same formatting requirements as the tokens, productions can only consist of lowercase letters and underscores (lower snake-case).

```
addition = addition ADD number;
addition = number;
```
As seen above, productions can also recursively contain themselves.
Further, empty productions are also allowed. These are mostly useful for termination of a recursive production:
```
program = program statement;
program = ;
```

### Parser entry production
The section above should enable you to write you own grammars for this Palex. However, before the parser can start parsing your provided grammar, it still needs to know which production to start with. This is where the special production `$S` comes into play. This is the so called **entry production**. The generated parser will always try to parse this production. To parse a series of additions, one could therefore write:
```
$S = program;
program = program addition;
program = ;
addition = addition ADD number;
addition = number;
number = INTEGER;
```
Note that the entry production can not recurse on itself. We therefore need another production (program) to do that for us. An input sequence that can't be reduced by an entry production is considered invalid by the parser.

### Production tags
By default, all productions that produce the same nonterminal type have the same reduce method. In case you want to split it into multiple ones, you can use tags:
```
$S = addition;
addition#recursive_case = addition ADD number;
addition#base_case = number;
number = INTEGER;
```

Of course, multiple productions of the same type can share a tag, like so:
```
binary_expr#recursive = binary_expr ADD number;
binary_expr#recursive = binary_expr SUB number;
number = INTEGER;
```
You can also reuse the same tag for different nonterminals / productions.

## Command line arguments
Palex expects a sequence of rule files as arguments. In addition, the following arguments that can also be passed to Palex:

### Options
| Option | Required | Default Value | Description |
| :----------------------: | :-----------------------------: | :-----: | :----------------------------------------------------------------------: |
| `-output-path <path>`    | No                              | `.`     | The output folder for the parser and lexer files.                        |
| `-util-path <path>`      | No                              | `.`     | The output folder for all util files.                                    |
| `-lang <C++/CPP>`        | Yes                             | None    | The target programming language.                                         |
| `-parser-type <LR/LALR>` | When the `--parser` flag is set | None    | The type of the generated parsers.                                       |
| `-lookahead <uint>`      | No                              | `0`     | Specifies the number of lookahead tokens for the parsers (integer >= 0). |
| `-module-name <name>`    | No                              | `palex` | The name of the module/namespace the generated code resides in.          |

### Flags

| Flag | Description |
| :----------: | :--------------------------------------: |
| `--lexer`    | Enables lexer generation.                |
| `--parser`   | Enables parser generation.               |
| `--util`     | Enables the generation of utility files. |
| `--fallback` | Enables token fallback for lexers.       |

Furthermore, you can also use `palex --version` to get the used palex version and `palex --help` to show the table above.