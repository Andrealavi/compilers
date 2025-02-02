# Python Grep Implementation

A Python implementation of a simplified grep-like utility using Thompson's NFA construction algorithm for regular expression pattern matching.

## Description

This program implements a subset of grep functionality using Nondeterministic Finite Automata (NFA) for pattern matching. It supports basic regular expression operations including concatenation, alternation (|), repetition (*, +), and optional characters (?). The implementation follows Thompson's construction algorithm to convert regular expressions into NFAs and uses them for pattern matching.

## Installation

No additional dependencies are required beyond Python 3.x. Simply download the `grep.py` file and run it from the command line.

## Usage

```bash
python grep.py [options] pattern [file...]
```

### Options

- `-l, --lines`: Show entire lines containing matches instead of just the matches
- `--no-color`: Disable colored output of matches

### Input Sources

The program can read input from:
- One or more files specified as command-line arguments
- Standard input (stdin) when no files are specified

### Examples

1. Search for a pattern in a file:
```bash
python grep.py "pattern" file.txt
```

2. Search in multiple files:
```bash
python grep.py "pattern" file1.txt file2.txt
```

3. Show full lines containing matches:
```bash
python grep.py -l "pattern" file.txt
```

4. Read from stdin:
```bash
echo "some text" | python grep.py "pattern"
```

5. Disable colored output:
```bash
python grep.py --no-color "pattern" file.txt
```

## Theoretical Background

### Regular Expressions and Thompson's Construction

The implementation is based on Ken Thompson's 1968 paper "Regular Expression Search Algorithm," which describes an efficient method for converting regular expressions into NFAs. Thompson's algorithm guarantees:
- Linear-time construction of the NFA from the regular expression
- Linear-space representation of the NFA
- Linear-time simulation of the NFA on input text

The key insight of Thompson's construction is that each regular expression operator can be converted into a small NFA with a specific structure, and these NFAs can be combined to create a larger NFA that recognizes the entire pattern.

### Shunting Yard Algorithm

The program uses Dijkstra's Shunting Yard algorithm to convert regular expressions from infix notation (the usual human-readable form) to postfix notation. This conversion is necessary because:
- It eliminates the need for parentheses
- It makes the operator precedence explicit
- It simplifies the construction of the Abstract Syntax Tree (AST)

The algorithm handles operator precedence and associativity using two main data structures:
- An output queue for the final postfix expression
- An operator stack for temporary storage of operators

### Finite State Automata

The program uses Nondeterministic Finite Automata (NFA) for pattern matching. Key characteristics of the NFA implementation include:
- States connected by labeled transitions (for input characters) and ε-transitions (empty transitions)
- Multiple active states during simulation
- Ability to transition to multiple states on a single input

## Implementation Details

### Code Structure

The implementation is organized into several key components:

1. **NFA Class**
   - Represents the NFA structure
   - Maintains lists of states and transitions
   - Provides methods for adding states and transitions

2. **AST Classes**
   - `RootAST`: Abstract base class for all AST nodes
   - `ExprAST`: Base class for expression nodes
   - `BinaryExprAST`: Handles concatenation and alternation
   - `UnaryExprAST`: Handles *, +, and ? operators
   - `CharExprAST`: Represents literal characters

3. **Regex Processing Pipeline**
   ```
   Input Regex → Mangled Regex → Postfix Notation → AST → NFA → Pattern Matching
   ```

### Key Algorithms

1. **Regex Mangling**
   - Inserts explicit concatenation operators between adjacent operands
   - Makes the parsing unambiguous

2. **NFA Construction**
   - Creates states and transitions based on the AST
   - Implements Thompson's construction rules for each operator

3. **NFA Simulation**
   - Maintains sets of current and next states
   - Handles both character transitions and ε-transitions
   - Implements efficient state tracking

4. **Pattern Matching**
   - Finds all matches in the input text
   - Handles overlapping matches
   - Supports both match-only and full-line output

### Performance Considerations

- The NFA simulation has linear time complexity with respect to the input text length
- State transitions are handled efficiently using lists
- Overlapping matches are resolved by keeping the longest match at each position
