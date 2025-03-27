# Parser

This document explains the implementation of the SQL parser.

## File structure
The parser is implemented in the `src/parser` directory.

- `parser.h, parser.cc`: Contains the parser implementation. These files should be used externally.
- `sql.h, sql.cc`: Defines structures for the parsed syntax tree.
- `bison.y`: Defines the parser using Bison.
- `flex.l:` Implements the lexical analyzer using Flex.

## About Bison and Flex

Documentation can be found at the following links:

- [Flex Documentation](https://westes.github.io/flex/manual/index.html#Top)
- [Bison Documentation](https://www.gnu.org/software/bison/manual/bison.html)

Below are some useful links and comments:

- [Writing Patterns in Flex](https://westes.github.io/flex/manual/Patterns.html#Patterns)

- [Declaration of Tokens (terminal symbols), nonterminal symbols, and syntac parsing with precedence in Bison](https://www.gnu.org/software/bison/manual/bison.html#Decl-Summary)

- Both Bison and Flex are configured to be reentrant, meaning the parser is thread-safe. If not reentrant, global variables would be used, making the parser non-thread-safe. This option ensures thread safety.
    - [Flex Reentrant Lexer](https://westes.github.io/flex/manual/Reentrant.html#Reentrant)
    - [Bison Pure Parser](https://www.gnu.org/software/bison/manual/bison.html#Pure-Decl)

- 


