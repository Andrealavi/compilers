import unittest
from parser import (
    Parser,
    first,
    follow,
    get_terminals_from_grammar,
    read_grammar,
    get_terminals_from_grammar
)

class TestGrammarFunctions(unittest.TestCase):
    def setUp(self):
        self.grammar = read_grammar("./grammar.txt")

    def test_read_grammar(self):
        self.assertEqual(self.grammar, {'E': ['TW', 'E$'], 'W': ['+TW', 'ɛ'], 'T': ['FS'], 'S': ['*FS', 'ɛ'], 'F': ['(E)', 'n']})

    def test_first(self):
        self.assertEqual(first("E", self.grammar), {"n", "("})
        self.assertEqual(first("S", self.grammar), {'*', 'ɛ'})

        with self.assertRaises(ValueError):
            first("G", self.grammar)

    def test_get_terminals(self):
        self.assertEqual(get_terminals_from_grammar(self.grammar), ['$', '+', 'ɛ', '*', 'ɛ', '(', ')', 'n'])

    def test_follow(self):
        self.assertEqual(follow("E", self.grammar), {')', '$'})
        self.assertEqual(follow("T", self.grammar), {')', '$', '+'})

        with self.assertRaises(ValueError):
            follow("G", self.grammar)


class TestParser(unittest.TestCase):
    def setUp(self):
        self.grammar = read_grammar("./grammar.txt")
        self.parser = Parser(self.grammar)

    def test_parsing_table(self):
        parsing_table = {
            'E': {'(': 'TW', 'n': 'TW'},
            'W': {'+': '+TW', '$': 'ɛ', ')': 'ɛ'},
            'T': {'(': 'FS', 'n': 'FS'},
            'S': {'*': '*FS', '+': 'ɛ', '$': 'ɛ', ')': 'ɛ'},
            'F': {'(': '(E)', 'n': 'n'}
        }

        self.parser.generate_parsing_table()

        self.assertEqual(self.parser.parsing_table, parsing_table)

    def test_parse(self):
        self.parser.generate_parsing_table()

        test_cases = [
            ('n', True),
            ('n+n', True),
            ('n*(n)', True),
            ('', False),
            ('(n*n', False),
            ('n+n)', False),
        ]

        for input_str, expected in test_cases:
            with self.subTest(input_str=input_str):
                self.assertEqual(self.parser.parse(input_str), expected)

if __name__ == '__main__':
    unittest.main()
