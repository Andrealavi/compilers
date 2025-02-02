from abc import ABC, abstractmethod
import argparse
import sys
from typing import List, Tuple
import re

# ANSI escape codes for text coloring
RED = '\033[91m'    # Red color for matches
RESET = '\033[0m'   # Reset color to default

# Operator precedence for regex operations
PRECEDENCE = {
    "|": 1,  # Alternation (lowest precedence)
    ".": 2,  # Concatenation
    "+": 3,  # One or more
    "*": 3,  # Zero or more
    "?": 3   # Zero or one (highest precedence)
}

# Operator associativity (left or right)
ASSOCIATIVITY = {
    "|": "l",  # Alternation is left-associative
    ".": "l",  # Concatenation is left-associative
    "+": "r",  # One or more is right-associative
    "*": "r",  # Zero or more is right-associative
    "?": "r"   # Zero or one is right-associative
}

class NFA:
    """
    Nondeterministic Finite Automaton implementation.
    Represents states and transitions for pattern matching.
    """
    def __init__(self):
        self.num_states = 0
        self.input_chars: list[str] = []     # Input characters for transitions
        self.first_states: list[int] = []    # First target state for transitions
        self.second_states: list[int] = []   # Second target state for ε-transitions

    def add_state(self) -> int:
        """
        Adds a new state to the NFA.
        Returns: The index of the newly created state
        """
        self.num_states += 1
        self.input_chars.append("")
        self.first_states.append(-1)
        self.second_states.append(-1)
        return self.num_states - 1

    def add_transition(self,
        state: int,
        input_char: str,
        first_state: int,
        second_state = -1):
        """
        Adds a transition from one state to another.

        Args:
            state: Source state index
            input_char: Character triggering the transition (or ε for epsilon transitions)
            first_state: Target state index
            second_state: Optional second target state for non-deterministic transitions
        """
        if (state < 0 or state > self.num_states):
            raise ValueError("State value must be between 0" +
                f" and the number of states ({self.num_states})")

        self.input_chars[state] = input_char
        self.first_states[state] = first_state
        self.second_states[state] = second_state


# Abstract base classes for AST nodes
class RootAST(ABC):
    """Base class for all AST nodes"""
    @abstractmethod
    def codegen(self, automaton: NFA) -> tuple[int, int]:
        """Generate NFA states and transitions for this node"""
        pass


class ExprAST(RootAST, ABC):
    """Base class for expression AST nodes"""
    @abstractmethod
    def codegen(self, automaton: NFA) -> tuple[int, int]:
        """Generate NFA states and transitions for this expression"""
        pass


class BinaryExprAST(ExprAST):
    """AST node for binary operations (concatenation and alternation)"""
    def __init__(self, op: str, LHS: ExprAST, RHS: ExprAST):
        self.op = op          # Operator ("." for concatenation, "|" for alternation)
        self.LHS = LHS        # Left-hand side expression
        self.RHS = RHS        # Right-hand side expression

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        """
        Generate NFA states and transitions for binary operations.
        Returns tuple of (initial_state, final_state)
        """
        initial_state = automaton.add_state()

        # Generate states for both operands
        lhs_initial_state, lhs_final_state = self.LHS.codegen(automaton)
        rhs_initial_state, rhs_final_state = self.RHS.codegen(automaton)

        final_state = automaton.add_state()

        # Handle concatenation
        if (self.op == "."):
            automaton.add_transition(initial_state, "ɛ", lhs_initial_state)
            automaton.add_transition(lhs_final_state, "ɛ", rhs_initial_state)
            automaton.add_transition(rhs_final_state, "ɛ", final_state)
        # Handle alternation
        elif (self.op == "|"):
            automaton.add_transition(initial_state, "ɛ", lhs_initial_state, rhs_initial_state)
            automaton.add_transition(final_state, "ɛ", lhs_final_state, rhs_final_state)

        return (initial_state, final_state)


class UnaryExprAST(ExprAST):
    """AST node for unary operations (*, +, ?)"""
    def __init__(self, op: str, expr: ExprAST):
        self.op = op      # Operator (*, +, or ?)
        self.expr = expr  # Expression being modified

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        """
        Generate NFA states and transitions for unary operations.
        Returns tuple of (initial_state, final_state)
        """
        initial_state = automaton.add_state()
        expr_initial_state, expr_final_state = self.expr.codegen(automaton)
        final_state = automaton.add_state()

        # Handle zero or more (*)
        if (self.op == "*"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state, final_state)
            automaton.add_transition(expr_final_state, "ɛ", expr_initial_state, final_state)
        # Handle zero or one (?)
        elif (self.op == "?"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state, final_state)
            automaton.add_transition(expr_final_state, "ɛ", final_state)
        # Handle one or more (+)
        elif (self.op == "+"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state)
            automaton.add_transition(expr_final_state, "ɛ", expr_initial_state, final_state)

        return (initial_state, final_state)


class CharExprAST(ExprAST):
    """AST node for literal characters"""
    def __init__(self, char: str):
        self.char = char  # The literal character

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        """
        Generate NFA states and transitions for a literal character.
        Returns tuple of (initial_state, final_state)
        """
        initial_state = automaton.add_state()
        final_state = automaton.add_state()
        automaton.add_transition(initial_state, self.char, final_state)
        return (initial_state, final_state)


def mangle_regex(regex: str) -> str:
    """
    Insert concatenation operators (.) between adjacent literal characters.
    This makes the regex parsing unambiguous.

    Args:
        regex: Input regular expression

    Returns:
        Mangled regex with explicit concatenation operators
    """
    regex_lenght = len(regex)
    mangled_regex: list[str] = []

    for i in range(regex_lenght - 1):
        mangled_regex.append(regex[i])
        # Add concatenation operator between letters
        if ((regex[i] != '(' and regex[i] != "|") and (regex[i+1].isalpha() or regex[i+1] == "(")):
            mangled_regex.append(".")

    mangled_regex.append(regex[-1])
    return "".join(mangled_regex)


def to_postfix(regex: str) -> list[str]:
    """
    Convert infix regex notation to postfix notation using Dijkstra's Shunting Yard algorithm.

    Args:
        regex: Input regular expression in infix notation

    Returns:
        List of characters representing the regex in postfix notation
    """
    output_queue: list[str] = []
    operator_stack: list[str] = []

    for char in regex:
        # Handle operands (literal characters)
        if char.isalnum():
            output_queue.append(char)
        # Handle operators
        elif char in PRECEDENCE.keys():
            operator_stack_lenght = len(operator_stack)
            i = 0 if operator_stack_lenght == 0 else operator_stack_lenght - 1

            # Process operators according to precedence and associativity
            while (operator_stack and operator_stack[i] != "(" and
                (PRECEDENCE[operator_stack[i]] > PRECEDENCE[char] or
                    (PRECEDENCE[operator_stack[i]] < PRECEDENCE[char])
                        and ASSOCIATIVITY[char] == 'l')):
                output_queue.append(operator_stack.pop())
                i -= 1

            operator_stack.append(char)
        # Handle opening parenthesis
        elif char == "(":
            operator_stack.append(char)
        # Handle closing parenthesis
        elif char == ")":
            top_stack = operator_stack.pop()

            # Pop operators until matching parenthesis is found
            while (top_stack != "(" and operator_stack):
                output_queue.append(top_stack)
                top_stack = operator_stack.pop()

            if not operator_stack and top_stack != "(":
                raise ValueError("Parentheses must be rightly balanced")

    # Pop remaining operators
    while (operator_stack):
        output_queue.append(operator_stack.pop())

    return output_queue


def to_AST(postfix_regex: list[str]) -> ExprAST:
    """
    Convert postfix regex notation to Abstract Syntax Tree.

    Args:
        postfix_regex: List of characters in postfix notation

    Returns:
        Root node of the AST
    """
    expr_stack = []

    for char in postfix_regex:
        # Handle literal characters
        if char.isalnum():
            expr_stack.append(CharExprAST(char))
        # Handle binary operators
        elif char == "|" or char == ".":
            RHS = expr_stack.pop()
            LHS = expr_stack.pop()
            expr_stack.append(BinaryExprAST(char, LHS, RHS))
        # Handle unary operators
        elif char == "*" or char == "+" or char == "?":
            expr = expr_stack.pop()
            expr_stack.append(UnaryExprAST(char, expr))

    return expr_stack.pop()


def simulate_NFA(automaton: NFA, input_string: str) -> bool:
    """
    Simulate NFA execution on input string.

    Args:
        automaton: NFA to simulate
        input_string: Input string to test

    Returns:
        True if string matches pattern, False otherwise
    """
    current_list: list[int] = [0]  # List of current active states
    next_list: list[int] = []      # List of states to process in next step

    # Process each character in input string
    for char in input_string:
        for state in current_list:
            # Handle character transitions
            if (automaton.input_chars[state] == char):
                # Add valid transitions to next states list
                if (automaton.first_states[state] not in next_list and automaton.first_states[state] >= 0):
                    next_list.append(automaton.first_states[state])
                if (automaton.second_states[state] not in next_list and automaton.second_states[state] >= 0):
                    next_list.append(automaton.second_states[state])
            # Handle epsilon transitions
            elif (automaton.input_chars[state] == "ɛ"):
                if (automaton.first_states[state] not in current_list and automaton.first_states[state] >= 0):
                    current_list.append(automaton.first_states[state])
                if (automaton.second_states[state] not in current_list and automaton.second_states[state] >= 0):
                    current_list.append(automaton.second_states[state])

        # Prepare for next character
        current_list = next_list
        next_list = []

    # Process remaining epsilon transitions after all input is consumed
    for state in current_list:
        if (automaton.input_chars[state] == "ɛ"):
            if (automaton.first_states[state] not in current_list and automaton.first_states[state] >= 0):
                current_list.append(automaton.first_states[state])
            if (automaton.second_states[state] not in current_list and automaton.second_states[state] >= 0):
                current_list.append(automaton.second_states[state])

    # Check if we reached an accepting state
    return (automaton.num_states - 2) in current_list


def find_matches(automaton: NFA, text: str) -> List[Tuple[int, int]]:
    """
    Find all matches of the pattern in the text.

    This function implements the pattern matching algorithm by trying all possible
    substrings of the input text and keeping track of non-overlapping matches.

    Args:
        automaton: Compiled NFA for the pattern
        text: Input text to search

    Returns:
        List[Tuple[int, int]]: List of (start, end) positions for each match
    """
    matches = []
    text_length = len(text)

    # Try all possible substrings
    for start in range(text_length):
        for end in range(start + 1, text_length + 1):
            if simulate_NFA(automaton, text[start:end]):
                matches.append((start, end))

    # Remove overlapping matches by keeping the longest match at each position
    filtered_matches = []
    i = 0
    while i < len(matches):
        current_start, current_end = matches[i]
        j = i + 1
        # Skip overlapping matches
        while j < len(matches) and matches[j][0] < current_end:
            j += 1
        filtered_matches.append((current_start, current_end))
        i = j

    return filtered_matches


def process_text(pattern: str, text: str, show_line_matches: bool = False, color: bool = True) -> str:
    """
    Process text and return matches with optional coloring and line context.

    This function handles the main text processing pipeline:
    1. Compiles the pattern into an NFA
    2. Finds matches in the text
    3. Formats the output according to the specified options

    Args:
        pattern: Regular expression pattern to search for
        text: Input text to search
        show_line_matches: If True, show entire lines containing matches
        color: If True, colorize matches in output

    Returns:
        str: Formatted string containing matches or matching lines
    """
    # Create NFA from pattern
    mangled_pattern = mangle_regex(pattern)
    postfix_pattern = to_postfix(mangled_pattern)
    ast = to_AST(postfix_pattern)
    automaton = NFA()
    ast.codegen(automaton)

    result = []

    # Process text line by line
    for line in text.split('\n'):
        matches = find_matches(automaton, line)
        if matches:
            if show_line_matches:
                # Show entire line with colored matches
                current_pos = 0
                line_result = ""
                for start, end in matches:
                    # Add text before match
                    line_result += line[current_pos:start]
                    # Add colored match
                    match_text = line[start:end]
                    line_result += f"{RED}{match_text}{RESET}" if color else match_text
                    current_pos = end
                # Add remaining text after last match
                line_result += line[current_pos:]
                result.append(line_result)
            else:
                # Show only the matches
                for start, end in matches:
                    match_text = line[start:end]
                    result.append(f"{RED}{match_text}{RESET}" if color else match_text)

    return '\n'.join(result)


def main():
    """
    Main entry point of the program.

    Handles command-line argument parsing and orchestrates the pattern matching
    process. Supports reading from files or stdin and provides options for
    output formatting.
    """
    # Set up command-line argument parser
    parser = argparse.ArgumentParser(
        description='A simplified grep implementation using NFA-based pattern matching')
    parser.add_argument('pattern',
        help='Regular expression pattern to search for')
    parser.add_argument('files', nargs='*',
        help='Files to search in (reads from stdin if no files provided)')
    parser.add_argument('-l', '--lines', action='store_true',
        help='Show entire lines containing matches instead of just matches')
    parser.add_argument('--no-color', action='store_true',
        help='Disable colored output')

    args = parser.parse_args()

    # Handle input from files or stdin
    if not args.files:
        # Read from stdin when no files are specified
        try:
            text = sys.stdin.read()
            print(process_text(args.pattern, text, args.lines, not args.no_color))
        except KeyboardInterrupt:
            # Handle Ctrl+C gracefully
            sys.exit(1)
        except Exception as e:
            print(f"Error processing stdin: {str(e)}", file=sys.stderr)
            sys.exit(1)
    else:
        # Process each specified input file
        for file_path in args.files:
            try:
                with open(file_path, 'r') as file:
                    text = file.read()
                    # Print file header if processing multiple files
                    if len(args.files) > 1:
                        print(f"\n==> {file_path} <==")
                    print(process_text(args.pattern, text, args.lines, not args.no_color))
            except FileNotFoundError:
                print(f"Error: File '{file_path}' not found", file=sys.stderr)
            except Exception as e:
                print(f"Error processing file '{file_path}': {str(e)}", file=sys.stderr)


if __name__ == "__main__":
    main()
