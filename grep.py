from abc import ABC, abstractmethod

PRECEDENCE = {
    "|": 1,
    ".": 2,
    "+": 3,
    "*": 3,
    "?": 3
}

ASSOCIATIVITY = {
    "|": "l",
    ".": "l",
    "+": "r",
    "*": "r",
    "?": "r"
}

# NFA - Class for representing Nondeterministic finite automata
class NFA:
    def __init__(self):
        self.num_states = 0
        self.input_chars: list[str] = []
        self.first_states: list[int] = []
        self.second_states: list[int] = []
        self.initial_state = 0

    def add_state(self) -> int:
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

        if (state < 0 or state > self.num_states):
            raise ValueError("State value must be between 0" +
                f" and the number of states ({self.num_states})")

        self.input_chars[state] = input_char
        self.first_states[state] = first_state
        self.second_states[state] = second_state


# AST Nodes definition
class RootAST(ABC):
    @abstractmethod
    def codegen(self, automaton: NFA) -> tuple[int, int]:
        pass


class ExprAST(RootAST, ABC):
    @abstractmethod
    def codegen(self, automaton: NFA) -> tuple[int, int]:
        pass

class BinaryExprAST(ExprAST):
    def __init__(self, op: str, LHS: ExprAST, RHS: ExprAST):
        self.op = op
        self.LHS = LHS
        self.RHS = RHS

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        initial_state = automaton.add_state()

        lhs_initial_state, lhs_final_state = self.LHS.codegen(automaton)
        rhs_initial_state, rhs_final_state = self.RHS.codegen(automaton)

        final_state = automaton.add_state()

        if (self.op == "."):
            automaton.add_transition(initial_state, "ɛ", lhs_initial_state)
            automaton.add_transition(lhs_final_state, "ɛ", rhs_initial_state)
            automaton.add_transition(rhs_final_state, "ɛ", final_state)
        elif (self.op == "|"):
            automaton.add_transition(initial_state, "ɛ", lhs_initial_state, rhs_initial_state)
            automaton.add_transition(final_state, "ɛ", lhs_final_state, rhs_final_state)

        return (initial_state, final_state)


class UnaryExprAST(ExprAST):
    def __init__(self, op: str, expr: ExprAST):
        self.op = op
        self.expr = expr

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        initial_state = automaton.add_state()

        expr_initial_state, expr_final_state = self.expr.codegen(automaton)

        final_state = automaton.add_state()

        if (self.op == "*"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state, final_state)
            automaton.add_transition(expr_final_state, "ɛ", expr_initial_state, final_state)
        elif (self.op == "?"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state, final_state)
            automaton.add_transition(expr_final_state, "ɛ", final_state)
        elif (self.op == "+"):
            automaton.add_transition(initial_state, "ɛ", expr_initial_state)
            automaton.add_transition(expr_final_state, "ɛ", expr_initial_state, final_state)

        return (initial_state, final_state)


class CharExprAST(ExprAST):
    def __init__(self, char: str):
        self.char = char

    def codegen(self, automaton: NFA) -> tuple[int, int]:
        initial_state = automaton.add_state()
        final_state = automaton.add_state()

        automaton.add_transition(initial_state, self.char, final_state)

        return (initial_state, final_state)


# Function that mangles the regex by adding a dot between simple characters
def mangle_regex(regex: str) -> str:
    regex_lenght = len(regex)

    mangled_regex: list[str] = []

    for i in range(regex_lenght - 1):
        mangled_regex.append(regex[i])

        if ((regex[i] != '(' and regex[i] != "|") and regex[i+1].isalpha()):
            mangled_regex.append(".")

    mangled_regex.append(regex[-1])

    return "".join(mangled_regex)

# Function that converts the mangled regex to its postfix notation
# It uses the shunting yard algorithm developed by Dijkstra
# Returns it as a list of characters
def to_postfix(regex: str) -> list[str]:
    output_queue: list[str] = []
    operator_stack: list[str] = []

    for char in regex:
        if char.isalnum():
            output_queue.append(char)
        elif char in PRECEDENCE.keys():
            operator_stack_lenght = len(operator_stack)
            i = 0 if operator_stack_lenght == 0 else operator_stack_lenght - 1

            while (operator_stack and operator_stack[i] != "(" and
                (PRECEDENCE[operator_stack[i]] > PRECEDENCE[char] or
                    (PRECEDENCE[operator_stack[i]] < PRECEDENCE[char])
                        and ASSOCIATIVITY[char] == 'l')):
                output_queue.append(operator_stack.pop())

                i -= 1

            operator_stack.append(char)
        elif char == "(":
            operator_stack.append(char)
        elif char == ")":
            top_stack = operator_stack.pop();

            while (top_stack != "(" and operator_stack):
                output_queue.append(top_stack)
                top_stack = operator_stack.pop()

            if not operator_stack and top_stack != "(":
                raise ValueError("Parentheses must be rightly balanced")

    while (operator_stack):
        output_queue.append(operator_stack.pop())

    return output_queue

# Converts the postfix notation to AST
def to_AST(postfix_regex: list[str]) -> ExprAST:
    expr_stack = []

    for char in postfix_regex:
        if char.isalnum():
            expr_stack.append(CharExprAST(char))
        elif char == "|" or char == ".":
            RHS = expr_stack.pop()
            LHS = expr_stack.pop()

            expr_stack.append(BinaryExprAST(char, LHS, RHS))
        elif char == "*" or char == "+" or char == "?":
            expr = expr_stack.pop()

            expr_stack.append(UnaryExprAST(char, expr))

    return expr_stack.pop()


def main():
    regex = input("insert the regex:")

    regex = mangle_regex(regex)

    root = to_AST(to_postfix(regex))

    automaton = NFA()
    root.codegen(automaton)

    print(automaton.input_chars)
    print(automaton.first_states)
    print(automaton.second_states)


if __name__ == "__main__":
    main()
