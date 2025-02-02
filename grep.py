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

def main():
    pass

if __name__ == "__main__":
    main()
