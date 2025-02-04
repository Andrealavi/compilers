# LL(1) Parser Implementation

A Python implementation of a recursive descent parser for LL(1) grammars. This project provides functionality to parse input strings according to a specified grammar, including the generation of parsing tables and computation of FIRST and FOLLOW sets.

## Features

- Grammar parsing from text files
- FIRST and FOLLOW set computation
- LL(1) parsing table generation
- Interactive and single-input parsing modes
- Comprehensive unit test coverage

## Theory and Implementation

### Grammar Format

The grammar should be provided in a text file with production rules in the format:
```
NT -> production
```
where `NT` is a non-terminal and `production` is a sequence of terminals and non-terminals. Multiple productions for the same non-terminal should be on separate lines.

Example grammar:
```
E -> TW
E -> E$
W -> +TW
W -> ɛ
T -> FS
S -> *FS
S -> ɛ
F -> (E)
F -> n
```

### Key Components

#### FIRST Set Computation
The `first()` function computes the FIRST set for a given phrase in the grammar. The FIRST set of a grammar symbol is the set of terminals that can appear as the first symbol of any string derived from that symbol.

#### FOLLOW Set Computation
The `follow()` function computes the FOLLOW set for a given non-terminal in the grammar. The FOLLOW set of a non-terminal A is the set of terminals that can appear immediately to the right of A in some sentential form.

#### Parsing Table Generation
The `generate_parsing_table()` method creates an LL(1) parsing table using the FIRST and FOLLOW sets. This table is used to guide the parsing process and determine which production to use at each step.

## Usage

### Installation

No additional dependencies are required beyond Python 3.x.

### Command Line Interface

The parser can be used in two modes:

1. Single input mode:
```bash
python parser.py grammar.txt -i 'n*(n+n)'
```

2. Interactive mode:
```bash
python parser.py grammar.txt --interactive
```

## Testing

The project includes a comprehensive test suite in `test_parser.py`. The tests cover:
- Grammar reading functionality
- FIRST and FOLLOW set computation
- Parsing table generation
- String parsing

### Running Tests

To run the test suite:

```bash
python -m unittest test_parser.py
```

To run specific test cases:

```bash
python -m unittest test_parser.TestParser.test_parse
```

### Test Structure

The test suite is organized into two main test classes:

1. `TestGrammarFunctions`: Tests basic grammar operations
   - Grammar file reading
   - FIRST set computation
   - FOLLOW set computation
   - Terminal symbol extraction

2. `TestParser`: Tests the parser functionality
   - Parsing table generation
   - Input string parsing with various test cases

## Limitations

- The grammar must be LL(1)
- The implementation assumes the grammar is well-formed
- Empty strings must be represented as 'ɛ' in the grammar
