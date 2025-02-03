class Parser:
    def __init__(self, grammar: dict):
        self.grammar = grammar
        self.parsing_table = {}

    def parse(self, input_str: str) -> bool:
        return True

def get_terminals_from_grammar(grammar: dict) -> list[str]:
    return []

def read_grammar(filename) -> dict[str, list[str]]:
    grammar: dict[str, list[str]] = {}

    if filename != "":
        with open(filename, "r") as f:
            for line in f.readlines():
                head, tail = line.strip().split("->")

                if head not in grammar.keys():
                    grammar[head] = []

                grammar[head].append(tail)

    return grammar

def first(phrase: str, grammar: dict) -> list[str]:
    return []

def follow(nonterminal: str, grammar: dict) -> list[str]:
    return []

def main():
    print(read_grammar("prova.txt"))

if __name__ == "__main__":
    main()
