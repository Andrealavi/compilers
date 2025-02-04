class Parser:
    def __init__(self, grammar: dict):
        self.grammar = grammar
        self.parsing_table = {}

    def parse(self, input_str: str) -> bool:
        return True

def get_terminals_from_grammar(grammar: dict) -> list[str]:
    terminals: list[str] = []

    for tails in grammar.values():
        for tail in tails:
            for token in tail:
                if token not in grammar.keys():
                    terminals.append(token)

    return terminals

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

def first(phrase: str, grammar: dict) -> set[str]:
    first_set: set[str] = set()

    queue = []

    i = 0
    phrase_len = len(phrase)
    char = phrase[i]

    while i < phrase_len:
        if char in grammar.keys():
            [queue.insert(0, tail) for tail in grammar[char] if tail not in queue]

            if "ɛ" not in grammar[char]:
                break
        else:
            first_set.add(char)
            break

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


def follow(nonterminal: str, grammar: dict) -> list[str]:
    return []

def main():
    grammar = read_grammar("./prova2.txt")

    print(first("T", grammar))

if __name__ == "__main__":
    main()
