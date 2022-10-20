# PaLex
![GitHub CI](https://github.com/Creepsy/palex/actions/workflows/tests.yml/badge.svg)

PaLex is a parser and lexer generator written in C++. It allows the creation of lexers and parsers supporting UTF-8 input. The following languages are currently supported:

| Language | Lexer Generator | Parser Generator |
| :------: | :-------------: | :--------------: |
| C++      | Yes             | No               |

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
**TODO**


## Lexer Generator

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