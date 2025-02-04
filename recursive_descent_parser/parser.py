#!/usr/bin/env python3
"""
LL(1) Parser Implementation

This module implements a recursive descent parser for LL(1) grammars. It includes functionality
to generate parsing tables, compute FIRST and FOLLOW sets, and parse input strings according
to a given grammar.

The grammar should be provided in a text file with production rules in the format:
    NT -> production
where NT is a non-terminal and production is a sequence of terminals and non-terminals.
Multiple productions for the same non-terminal should be on separate lines.

Example grammar file:
    E -> T+E
    E -> T
    T -> F*T
    T -> F
    F -> (E)
    F -> n
"""

import argparse
import sys
from typing import Dict, List, Set


class Parser:
    """
    A recursive descent parser for LL(1) grammars.

    This class implements parsing functionality including parsing table generation
    and input validation against the grammar.

    Attributes:
        grammar (dict): The grammar rules with non-terminals as keys and lists of productions as values
        parsing_table (dict): The LL(1) parsing table generated from the grammar
    """

    def __init__(self, grammar: Dict[str, List[str]]):
        """
        Initialize the parser with a grammar.

        Args:
            grammar: A dictionary mapping non-terminals to their productions
        """
        self.grammar = grammar
        self.parsing_table = {}

    def generate_parsing_table(self) -> None:
        """
        Generate the LL(1) parsing table for the grammar.

        This method computes the parsing table entries using FIRST and FOLLOW sets.
        The table is used to guide the parsing process.

        Raises:
            ValueError: If the grammar is not LL(1) (i.e., if there are parsing conflicts)
        """
        non_terminals = self.grammar.keys()

        for non_terminal in non_terminals:
            self.parsing_table[non_terminal] = {}

            for tail in self.grammar[non_terminal]:
                if tail[-1] == "$":
                    continue

                tail_first = first(tail, self.grammar)

                if "ɛ" in tail_first:
                    non_terminal_follow = follow(non_terminal, self.grammar)

                    for terminal in non_terminal_follow:
                        if terminal in self.parsing_table[non_terminal].keys():
                            raise ValueError("The grammar is not LL(1)")

                        self.parsing_table[non_terminal][terminal] = tail

                    tail_first.remove("ɛ")

                for terminal in tail_first:
                    if terminal in self.parsing_table[non_terminal].keys():
                        raise ValueError("The grammar is not LL(1)")

                    if terminal != "$":
                        self.parsing_table[non_terminal][terminal] = tail
                    else:
                        self.parsing_table[non_terminal][terminal] = "accept"

    def parse(self, input_str: str) -> bool:
        """
        Parse an input string according to the grammar.

        Args:
            input_str: The input string to parse

        Returns:
            bool: True if the input string is valid according to the grammar, False otherwise
        """
        input_buffer = list(input_str.replace(" ", ""))

        symbol_stack = ["E"]  # Starting with the start symbol

        while input_buffer:
            if len(symbol_stack) == 0:
                return False

            symbol = symbol_stack.pop()

            if symbol in self.grammar.keys():
                try:
                    tail = list(self.parsing_table[symbol][input_buffer[0]])
                    tail.reverse()
                    for char in tail:
                        symbol_stack.append(char)
                except KeyError:
                    return False
            else:
                if symbol != "ɛ" and symbol != input_buffer.pop(0):
                    return False

        while symbol_stack:
            symbol = symbol_stack.pop()
            if symbol in self.grammar.keys() and "ɛ" not in self.grammar[symbol]:
                return False
            elif symbol not in self.grammar.keys():
                return False

        return True


def get_terminals_from_grammar(grammar: Dict[str, List[str]]) -> List[str]:
    """
    Extract all terminal symbols from the grammar.

    Args:
        grammar: The grammar to analyze

    Returns:
        A list of terminal symbols found in the grammar
    """
    terminals: List[str] = []

    for tails in grammar.values():
        for tail in tails:
            for token in tail:
                if token not in grammar.keys():
                    terminals.append(token)

    return terminals


def read_grammar(filename: str) -> Dict[str, List[str]]:
    """
    Read a grammar from a text file.

    Args:
        filename: Path to the grammar file

    Returns:
        A dictionary representing the grammar

    Raises:
        ValueError: If the filename is empty
        FileNotFoundError: If the file doesn't exist
    """
    grammar: Dict[str, List[str]] = {}

    if filename != "":
        with open(filename, "r") as f:
            for line in f.readlines():
                head, tail = line.replace(" ", "").strip().split("->")
                if head not in grammar.keys():
                    grammar[head] = []
                grammar[head].append(tail)
    else:
        raise ValueError("You must insert a valid file name")

    return grammar


def first(phrase: str, grammar: Dict[str, List[str]]) -> Set[str]:
    """
    Compute the FIRST set for a given phrase in the grammar.

    Args:
        phrase: The phrase to compute FIRST set for
        grammar: The grammar to use

    Returns:
        The FIRST set for the phrase

    Raises:
        ValueError: If an invalid symbol is encountered
    """
    first_set: Set[str] = set()
    queue = []
    terminals = get_terminals_from_grammar(grammar)

    i = 0
    phrase_len = len(phrase)
    char = phrase[i]

    while i < phrase_len:
        if char in grammar.keys():
            [queue.insert(0, tail) for tail in grammar[char] if tail not in queue]

            if "ɛ" not in grammar[char]:
                break
        elif char in terminals:
            first_set.add(char)
            break
        else:
            raise ValueError(f"{char} is neither a terminal nor a nonterminal char")

        i += 1

    if (i == phrase_len):
        first_set.add("ɛ")

    while queue:
        tail = queue.pop()
        for char in tail:
            if char in grammar.keys():
                [queue.insert(0, tail) for tail in grammar[char] if tail not in queue and tail[0] != char]
                if "ɛ" not in grammar[char]:
                    break
            else:
                first_set.add(char)
                break

    return first_set


def follow(nonterminal: str, grammar: Dict[str, List[str]]) -> Set[str]:
    """
    Compute the FOLLOW set for a given non-terminal in the grammar.

    Args:
        nonterminal: The non-terminal to compute FOLLOW set for
        grammar: The grammar to use

    Returns:
        The FOLLOW set for the non-terminal

    Raises:
        ValueError: If the symbol is not a non-terminal
    """
    follow_set = set()
    queue = [nonterminal]

    if nonterminal not in grammar.keys():
        raise ValueError(f"{nonterminal} is not a nonterminal symbol")

    while queue:
        symbol = queue.pop()
        for k, v in grammar.items():
            for tail in v:
                index = tail.find(symbol)
                if index > -1 and index != (len(tail) - 1):
                    [follow_set.add(char) for char in first(tail[index+1], grammar)]
                    if "ɛ" in follow_set:
                        follow_set.remove("ɛ")
                        queue.insert(0,k)
                elif index > -1 and k != symbol:
                    queue.insert(0, k)

    return follow_set


def interactive_mode(parser: Parser) -> None:
    """
    Run the parser in interactive mode, allowing users to input strings to parse.

    Args:
        parser: The configured Parser instance
    """
    print("Enter strings to parse (press Ctrl+D or Ctrl+C to exit):")
    try:
        while True:
            input_str = input("> ")
            if input_str.strip():
                result = parser.parse(input_str)
                print(result)
    except (EOFError, KeyboardInterrupt):
        print("\nExiting interactive mode...")


def main():
    """
    Main function to handle command-line usage of the parser.

    Supports both single input and interactive modes.
    """
    parser = argparse.ArgumentParser(
        description="LL(1) Parser - Parse input strings against a given grammar",
        epilog="Example: %(prog)s grammar.txt -i 'n*(n+n)'"
    )
    parser.add_argument('grammar_file', help='Path to the grammar definition file')
    parser.add_argument('-i', '--input', help='Input string to parse')
    parser.add_argument('--interactive', action='store_true',
                       help='Run in interactive mode')

    args = parser.parse_args()

    try:
        # Read and set up the grammar
        grammar = read_grammar(args.grammar_file)
        ll_parser = Parser(grammar)
        ll_parser.generate_parsing_table()

        if args.interactive:
            interactive_mode(ll_parser)
        elif args.input:
            result = ll_parser.parse(args.input)
            print(result)
        else:
            parser.print_help()
            sys.exit(1)

    except (ValueError, FileNotFoundError) as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
