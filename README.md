# PaLex
![GitHub CI](https://github.com/Creepsy/palex/actions/workflows/tests.yml/badge.svg)

PaLex is a parser and lexer generator written in C++. It allows the creation of lexers and parsers supporting UTF-8 input. The following languages are currently supported:

| Language | Lexer Generator | Parser Generator |
| :------: | :-------------: | :--------------: |
| C++      | Yes             | No               |

## Table of Contents
- [PaLex](#palex)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
    - [Creating a custom PaLex project](#creating-a-custom-palex-project)
  - [Config Files](#config-files)
    - [Language tags](#language-tags)
    - [Lexer Configurations](#lexer-configurations)
      - [General Configurations](#general-configurations)
      - [Configurations for C++ projects](#configurations-for-c-projects)
  - [Lexer Rules](#lexer-rules)
    - [Naming of the rule file](#naming-of-the-rule-file)
    - [Token rule syntax](#token-rule-syntax)
    - [Naming limitations](#naming-limitations)
    - [Regex limitations](#regex-limitations)
    - [Token ignore list](#token-ignore-list)
    - [Special tokens](#special-tokens)
    - [Token priority](#token-priority)
## Getting Started

In order to clone the repository and build PaLex run: 
```
git clone https://github.com/Creepsy/palex.git
cd palex
mkdir build && cd build
cmake .. && make
```

In case you want to try out the example or you already have a palex project, pass the path to the corresponding folder as argument to the program:

```
./palex ../examples
```

In case of the example project, the generated files can be found in the build folder.

### Creating a custom PaLex project
A PaLex project consists out of a folder containing a palex config file and the corresponding rule files. To get started with creating a custom PaLex project, create a folder containing a `palex.cfg` file. This file is **mandatory** and the generator won't work when provided with a path to a folder that doesn't contain such file on the top level directory.

This is the bare minimum a config file needs to contain in order to be recognized as valid by the program:

```json
{
    "language": "C++" // of course other languages are also possible
}
```

In order to add a custom lexer to your PaLex project, create a new entry called `lexers` which contains the configurations for all lexers of the project. To add a lexer, simply add an entry to `lexers`:

```json

{
    "language": "C++",
    "lexers": {
        "ExampleLexer": {
        }
    }
}
```

This config file registers a lexer called `ExampleLexer`. The generator expects a lexer rule file in the same directory with the name `ExampleLexer.lrules`. As no configs for the lexer are specified, the generator will use the default settings instead (**TODO** link to default config).

Here is a simple rule file for a lexer that we will use for this example:
```
INT = "int";
NUMBER = "[0-9]+";
ASSIGN = "=";
```

Your folder structure should now look like this:
- project_folder
  - palex.cfg
  - ExampleLexer.lrules

When supplying the project folder to the generator you should see four files popping up in your project folder:
- project_folder
  - palex.cfg
  - ExampleLexer.lrules
  - **ExampleLexer.h**
  - **ExampleLexer.cpp**
  - **utf8.h**          
  - **utf8.cpp**        

Congrats! You just created your first lexer with PaLex!

## Config Files
Every PaLex project needs a config file. This file **always** needs to be named `palex.cfg` and has to be located at the top level of the project folder.

Every config consists of a JSON-object that contains multiple fields:

| Field Name | Required | Type   | Usage |
| :--------: | :------: | :----: |:----- |
| `language` | Yes      | String | This tag specifies the programming language of the generated files. You can find a list of all supported language tags [here](#language-tags). |
| `lexers`   | No       | Object | This field contains all lexer configs of this PaLex project. Further information on lexer configs can be found [here](#lexer-configurations). |

### Language tags
All possible values for the `language` field:
| Language | Possible Tags      |
| :------: | :----------------- |
| C++      | C++, c++, CPP, cpp |

### Lexer Configurations
The name of a lexer is specified through it's entry key in `lexers`. The lexer configuration itself is also a JSON-Object. It's entries are the corresponding configurations for that specific lexer only. When a configuration is not defined, default settings are applied by default. However, some languages may require some fields to be explicitly specified.

Note, that the name of the lexer also defines the name of the corresponding rule file. The generator expects a file named like the lexer with the file extension `.lrules` in the same folder as the `palex.cfg` is. Otherwise the lexer will be skipped during generation. The generated files will also be named after the lexer.


#### General Configurations
There are a few general settings that are supported by every target language:

| Field Name       | Required | Type   | Default Value | Usage |
| :--------------: | :------: | :----: | :-----------: | :---: |
| `lexer_path`     | No       | String | `.`           | The output folder of the generated lexer |
| `token_fallback` | No       | Bool   | `true`           | If this option is enabled, the lexer will try to match a shorter token if it runs into a dead end while parsing a longer one (e.g. it will match `int` when trying to parse `integer` from `integn` instead of an undefined token) |

#### Configurations for C++ projects
Here are all options to further customize C++ lexers for reference:

| Field Name        | Required | Type   | Default Value     | Usage |
| :---------------: | :------: | :----: | :---------------: | :---: |
| `lexer_namespace` | No       | String | `palex`           | The namespace the generated lexer resides in |
| `utf8_lib_path`   | No       | String |  `.`           | Folder where the utf8 library is located |
| `create_utf8_lib` | No       | Bool   | `true`           | Specifies whether the utf8 library should also be generated |

## Lexer Rules

### Naming of the rule file
In order the be recognized by the parser, a lexer rule file has to have the file extension ".lrules". It has to be placed on the same level as the `palex.cfg` file. The file name is the same as the one of the lexer.

### Token rule syntax
A token is defined through the token name followed by an equal sign and the corresponding regex. Every token definition ends with a semicolon. A few valid examples for token definitions are:

```
INTEGER = "\d+";
INT = "int";
```

Token names should always be written with upper-case letters and underscores (upper snake-case). Please note that [some names are reserved](#naming-limitations) by the generator itself. Rules are allowed to use unicode characters, as long as the rule file is stored in UTF-8 format.
As the lexer isn't able to skip characters, all characters from the input stream have to be processed. However, there exists an option to [ignore specific tokens](#token-ignore-list).

### Naming limitations
Some token names are reserved by the generator itself, and therefore can't be used. These are: UNDEFINED, END_OF_FILE

### Regex limitations
This generator supports most of the commonly used regex features. Please note however, that the following widely used features are currently not supported:
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

### Token priority
The generator has an automatic priority system for token:

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

Keep in mind, that **priority levels always have to be >= 0**. In case you want to set the priority of an ignored token, the **ignore tag `$` has to stand before the priority tag**. 
