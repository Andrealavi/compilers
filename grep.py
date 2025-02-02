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

# AST Nodes definition
class RootAST(ABC):
    @abstractmethod
    def codegen(self, automaton):
        pass


class ExprAST(RootAST, ABC):
    @abstractmethod
    def codegen(self, automaton):
        pass

class BinaryExprAST(ExprAST):
    def __init__(self, op: str, LHS: ExprAST, RHS: ExprAST):
        self.op = op
        self.LHS = LHS
        self.RHS = RHS

    def codegen(self, automaton):
        pass

class UnaryExprAST(ExprAST):
    def __init__(self, op: str, expr: ExprAST):
        self.op = op
        self.expr = expr

    def codegen(self, automaton):
        pass

class CharExprAST(ExprAST):
    def __init__(self, char: str):
        self.char = char

    def codegen(self, automaton):
        pass

# Function that mangles the regex by adding a dot between simple characters
def mangle_regex(regex: str) -> str:
    regex_lenght = len(regex)

    mangled_regex: list[str] = []

    for i in range(regex_lenght - 1):
        mangled_regex.append(regex[i])

        if (regex[i].isalpha() and regex[i+1].isalpha()):
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


def main():
    regex = input("insert the regex:")

    regex = mangle_regex(regex)

    print(to_postfix(regex))

if __name__ == "__main__":
    main()
