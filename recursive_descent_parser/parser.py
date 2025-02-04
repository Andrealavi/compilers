class Parser:
    def __init__(self, grammar: dict):
        self.grammar = grammar
        self.parsing_table = {}

    def generate_parsing_table(self) -> None:
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
        input_buffer = list(input_str.replace(" ", ""))

        symbol_stack = ["E"]

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


def follow(nonterminal: str, grammar: dict) -> set[str]:
    follow_set = set()

    queue = [nonterminal]

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

def main():
    grammar = read_grammar("./grammar.txt")
    parser = Parser(grammar)

    parser.generate_parsing_table()

    print(parser.parse("n*(n*(n+n))"))

if __name__ == "__main__":
    main()
