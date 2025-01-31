# NFA to DFA Converter

This utility implements the subset construction algorithm to convert Nondeterministic Finite Automata (NFA) into Deterministic Finite Automata (DFA). The tool can generate DOT format files for both the input NFA and the resulting DFA, which can then be converted into SVG visualizations.

## Features

- Convert NFA to DFA using subset construction algorithm
- Generate DOT file representations of both NFA and DFA

## Installation

To compile the program, use a C++ compiler that supports C++17 or later:

```bash
clang++ -o subset_construction sc.cpp
```

## Usage

The basic syntax for running the utility is:

```bash
./subset_construction --input input_file [--output output_file] [-n]
```

Arguments:
- `--input input_file`: Required. Specifies the input file containing the NFA definition
- `--output output_file`: Optional. Specifies the output file for the DOT representation (defaults to "out.dot")
- `-n`: Optional flag. When present, generates the DOT file for the input NFA instead of the converted DFA

Example:
```bash
./subset_construction --input nfa.txt --output automaton.dot
```

## Input File Format

The input file must follow this specific format:

```
number_of_states
initial_state
input_char,first_state,second_state
input_char,first_state,second_state
...
final_states
```

Where:
- `number_of_states`: Integer specifying the total number of states in the NFA
- `initial_state`: Integer specifying the starting state (typically 0)
- For each state transition line:
  - `input_char`: Character for the transition (use '-' for ε-transitions)
  - `first_state`: Integer for the first target state (-1 if no transition)
  - `second_state`: Integer for the second target state (-1 if no transition)
- `final_states`: List of integers representing accepting states, terminated by -1

Example input file:
```
10          // Number of states
0           // Iniitial states
// Directed edges
-,1,4
a,2,-1
b,3,-1
-,9,-1
-,5,7
a,6,-1
-,5,7
c,8,-1
-,9,-1
 ,-1,-1
// Final states
9

```

## The Subset Construction Algorithm

The subset construction algorithm converts an NFA to a DFA through these steps:

1. **ε-closure Computation**: For each state, compute the set of states reachable through ε-transitions (including the state itself).

2. **Initial State Creation**: Create the initial state of the DFA as the ε-closure of the NFA's initial state.

3. **State Processing**:
  For each unprocessed DFA state and input symbol:
    - Compute the set of states reachable from the current subset on the given input
    - Compute the ε-closure of this set
    - If this creates a new subset of states, add it to the unprocessed states queue
    - Create a transition in the DFA from the current state to the state representing this subset

4. **Final States Determination**: A DFA state is accepting if any of its constituent NFA states is accepting.

The implementation in this utility follows these steps while maintaining a mapping between subsets of NFA states and their corresponding DFA states.

## Generating SVG Visualizations

To convert the generated DOT file to an SVG visualization, you'll need to have Graphviz installed. Use the following command:

```bash
dot -Tsvg input.dot -o output.svg
```

For example:
```bash
dot -Tsvg automaton.dot -o automaton.svg
```

This will create an SVG file that can be viewed in any web browser or vector graphics program.

## Visualizations Explained

The generated visualizations use the following conventions:
- States are represented as circles
- Accepting states are represented as double circles
- Transitions are shown as labeled arrows between states
- ε-transitions are labeled with "ɛ"
- States are numbered starting from 0
- The layout is left-to-right (specified by `rankdir=LR` in the DOT file)

## Examples

Given the example input file:
```
10
0
-,1,4
a,2,-1
b,3,-1
-,9,-1
-,5,7
a,6,-1
-,5,7
c,8,-1
-,9,-1
 ,-1,-1
9

```

You can generate both NFA and DFA visualizations:

For NFA:
```bash
./subset_construction --input example.txt --output nfa.dot -n
dot -Tsvg nfa.dot -o nfa.svg
```

For DFA:
```bash
./subset_construction --input example.txt --output dfa.dot
dot -Tsvg dfa.dot -o dfa.svg
```

## Troubleshooting

Common issues and their solutions:

1. **Input File Format Errors**: Ensure your input file strictly follows the format specified above. Each transition line must have exactly two comma-separated numbers following the input character.

2. **Invalid State Numbers**: State numbers must be non-negative integers less than the total number of states, or -1 to indicate no transition.

3. **DOT File Generation**: If the DOT file isn't generated, check that you have write permissions in the output directory and that the input file exists and is readable.

4. **SVG Conversion**: If the SVG conversion fails, ensure Graphviz is installed on your system. On most Unix-like systems, it can be installed via the package manager:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install graphviz

   # macOS with Homebrew
   brew install graphviz
   ```

## Implementation Details

The implementation consists of three main classes:
- `NFA`: Represents the nondeterministic finite automaton
- `DFA`: Represents the deterministic finite automaton
- `epsilonClosure()`: Helper function to compute ε-closures
- `subsetConstruction()`: Implements the main conversion algorithm

The code uses modern C++ features and the Standard Template Library (STL) for efficient data structures and algorithms.
