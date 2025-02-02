import unittest
from grep import (
    NFA,
    mangle_regex,
    to_postfix,
    to_AST,
    simulate_NFA,
    find_matches,
    process_text,
    CharExprAST,
    BinaryExprAST,
    UnaryExprAST
)

class TestNFA(unittest.TestCase):
    def setUp(self):
        self.nfa = NFA()

    def test_add_state(self):
        state = self.nfa.add_state()
        self.assertEqual(state, 0)
        self.assertEqual(self.nfa.num_states, 1)
        self.assertEqual(len(self.nfa.input_chars), 1)
        self.assertEqual(len(self.nfa.first_states), 1)
        self.assertEqual(len(self.nfa.second_states), 1)

    def test_add_transition(self):
        state0 = self.nfa.add_state()
        state1 = self.nfa.add_state()
        self.nfa.add_transition(state0, 'a', state1)
        self.assertEqual(self.nfa.input_chars[state0], 'a')
        self.assertEqual(self.nfa.first_states[state0], state1)
        self.assertEqual(self.nfa.second_states[state0], -1)

    def test_invalid_state_transition(self):
        with self.assertRaises(ValueError):
            self.nfa.add_transition(-1, 'a', 0)
        with self.assertRaises(ValueError):
            self.nfa.add_transition(100, 'a', 0)

class TestRegexParsing(unittest.TestCase):
    def test_mangle_regex(self):
        test_cases = [
            ('ab', 'a.b'),
            ('a|b', 'a|b'),
            ('abc', 'a.b.c'),
            ('a(bc)', 'a.(b.c)'),
        ]
        for input_regex, expected in test_cases:
            with self.subTest(input_regex=input_regex):
                self.assertEqual(mangle_regex(input_regex), expected)

    def test_to_postfix(self):
        test_cases = [
            ('a.b', ['a', 'b', '.']),
            ('a|b', ['a', 'b', '|']),
            ('a.b*', ['a', 'b', '*', '.']),
            ('(a|b).c', ['a', 'b', '|', 'c', '.']),
        ]
        for input_regex, expected in test_cases:
            with self.subTest(input_regex=input_regex):
                self.assertEqual(to_postfix(input_regex), expected)

class TestASTConstruction(unittest.TestCase):
    def test_char_expr(self):
        expr = CharExprAST('a')
        nfa = NFA()
        initial, final = expr.codegen(nfa)
        self.assertEqual(nfa.input_chars[initial], 'a')
        self.assertEqual(nfa.first_states[initial], final)

    def test_binary_expr(self):
        expr = BinaryExprAST('.', CharExprAST('a'), CharExprAST('b'))
        nfa = NFA()
        initial, final = expr.codegen(nfa)
        self.assertTrue(initial >= 0)
        self.assertTrue(final >= 0)

    def test_unary_expr(self):
        expr = UnaryExprAST('*', CharExprAST('a'))
        nfa = NFA()
        initial, final = expr.codegen(nfa)
        self.assertTrue(initial >= 0)
        self.assertTrue(final >= 0)

class TestPatternMatching(unittest.TestCase):
    def setUp(self):
        # Create a simple NFA for pattern 'a*b'
        pattern = mangle_regex('a*b')
        postfix = to_postfix(pattern)
        ast = to_AST(postfix)
        self.nfa = NFA()
        ast.codegen(self.nfa)

    def test_simulate_nfa(self):
        test_cases = [
            ('b', True),
            ('ab', True),
            ('aab', True),
            ('', False),
            ('a', False),
            ('ba', False),
        ]
        for input_str, expected in test_cases:
            with self.subTest(input_str=input_str):
                self.assertEqual(simulate_NFA(self.nfa, input_str), expected)

    def test_find_matches(self):
        text = "ab aab aaab"
        matches = find_matches(self.nfa, text)
        expected = [(0, 2), (3, 6), (7, 11)]
        self.assertEqual(matches, expected)

class TestTextProcessing(unittest.TestCase):
    def setUp(self):
        self.test_file = 'test_input.txt'
        with open(self.test_file, 'w') as f:
            f.write("Hello World\nThis is a test\nWorld of testing")

    def test_process_text(self):
        with open(self.test_file, 'r') as f:
            text = f.read()

        # Test simple word match
        result = process_text('World', text, show_line_matches=True, color=False)
        self.assertIn('Hello World', result)
        self.assertIn('World of testing', result)

        # Test pattern match
        result = process_text('te?st', text, show_line_matches=True, color=False)
        self.assertIn('This is a test', result)
        self.assertIn('testing', result)

    def tearDown(self):
        import os
        if os.path.exists(self.test_file):
            os.remove(self.test_file)

if __name__ == '__main__':
    unittest.main()
