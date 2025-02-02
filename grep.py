from abc import ABC, abstractmethod

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

def main():
    regex = input("insert the regex:")

    regex = mangle_regex(regex)

if __name__ == "__main__":
    main()
