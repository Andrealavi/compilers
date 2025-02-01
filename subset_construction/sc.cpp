#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <queue>
#include <map>
#include <set>
#include <vector>

/// DFA - Class that represents a Deterministic Finite Automaton
class DFA {
    private:
        int initialState = 0;
        int numStates = 0;
        std::vector<std::map<char, int>> transitions;
        std::set<int> finalStates;

    public:
        // Constructor
        DFA() {}

        // Getters
        std::set<int> getFinalStates() const { return finalStates; }

        std::vector<std::map<char, int>> getTransitions() const { return transitions; }

        std::optional<int> getTransition(int state, char input_char) const {
            if (state < 0 && state > numStates) {
                std::cerr << "State index out of range" << std::endl;
                return std::nullopt;
            }

            const auto it = transitions[state].find(input_char);

            if (it == transitions[state].end()) {
                return -1;
            }

            return it->second;
        }

        // Setters
        void addState() {
            numStates++;

            transitions.push_back(std::map<char,int>());
        }

        void addFinalState(int state) { finalStates.insert(state); }

        void setTransition(int state, char input_char, int transition_state) { transitions[state][input_char] = transition_state; }

        void toDOT(std::ostream& file) {
            file << "digraph DFA {" << std::endl;
            file << "rankdir=LR" << std::endl;
            file << "\"\" [shape=none, label=\"\"]" << std::endl;

            int state = 0;

            std::string nodeConfig;

            for (int i = 0; i < numStates; i++) {
                if (finalStates.find(i) != finalStates.end()) {
                    file << i << "[shape=doublecircle]" << std::endl;
                } else if (i == initialState) {
                    file << "\"\" -> " << i << std::endl;
                } else {
                    file << i << "[shape=circle]" << std::endl;
                }
            }

            for (auto stateTransitions : transitions) {
                for (auto const& [k, v] : stateTransitions) {
                    nodeConfig = (" [label=\"" + std::string{k} + "\"]");

                    file << state << " -> " << v << nodeConfig << std::endl;
                }

                state++;
            }

            file << "}" << std::endl;
        }
};

/// NFA - Class that represents a Nondeterministic Finite Automaton
class NFA {
    private:
        int numStates;
        int initialState = 0;
        std::vector<char> input_chars;
        std::vector<int> first_state;
        std::vector<int> second_state;
        std::set<int> finalStates;

    public:
        NFA() {}

        NFA(int numStates) : numStates(numStates) {}

        NFA(int initialState, int numStates) : initialState(initialState), numStates(numStates) {
            input_chars.resize(numStates);
            first_state.resize(numStates);
            second_state.resize(numStates);
        }

        NFA(std::vector<char> ic, std::vector<int> fs, std::vector<int> ss, std::set<int> finalStates)
            : input_chars(ic), first_state(fs), second_state(ss), finalStates(finalStates) { numStates = input_chars.size(); }

        // Getters
        int getInitialState() const { return initialState; }

        std::set<int> getFinalStates() const { return finalStates; }

        std::vector<int> getTransition(int state, char input_char) const {
            if (input_chars[state] == input_char) {
                std::vector<int> transitions = {first_state[state], second_state[state]};

                return transitions;
            }

            return {};
        }

        std::set<char> getAlphabet() const {
            std::set<char> alphabet;

            for (const char c : input_chars) {
                if (alphabet.find(c) == alphabet.end() && c != '-' && c != ' ') {
                    alphabet.insert(c);
                }
            }

            return alphabet;
        }

        // Setters
        bool loadState(std::ifstream& file) {
            file >> numStates;

            input_chars.resize(numStates);
            first_state.resize(numStates);
            second_state.resize(numStates);

            file >> initialState;

            std::string line;
            std::getline(file, line); // consume leftover newline

            for (int i = 0; i < numStates; i++) {
                std::getline(file, line);
                std::stringstream ss(line);
                std::string token;

                // Parse input_char,first_state,second_state
                std::getline(ss, token, ',');
                input_chars[i] = token[0];

                std::getline(ss, token, ',');
                first_state[i] = std::stoi(token);

                std::getline(ss, token, ',');
                second_state[i] = std::stoi(token);
            }

            int finalState;
            while (file >> finalState && finalState != -1) {
                finalStates.insert(finalState);
            }

            file.close();

            return true;
        }

        // Converts the NonDeterministic
        void toDOT(std::ostream& file) {
            file << "digraph NFA {" << std::endl;
            file << "rankdir=LR" << std::endl;
            file << "\"\" [shape=none, label=\"\"]" << std::endl;

            for (int i = 0; i < numStates; i++) {
                if (finalStates.find(i) != finalStates.end()) {
                    file << i << "[shape=doublecircle]" << std::endl;
                } else if (i == initialState) {
                    file << "\"\" -> " << i << std::endl;
                } else {
                    file << i << "[shape=circle]" << std::endl;
                }
            }

            for (int i = 0; i < numStates; i++) {
                std::string label;

                if (input_chars[i] == '-') {
                    label = "ɛ";
                } else {
                    label = input_chars[i];
                }

                if (first_state[i] >= 0) file << i << " -> " << first_state[i] << " [label=\"" << label << "\"]" << std::endl;
                if (second_state[i] >= 0) file << i << " -> " << second_state[i] << " [label=\"" << label << "\"]" << std::endl;
            }

            file << "}" << std::endl;
        }
};

/// Functions that computes the ɛ-closure of a set of states
std::set<int> epsilonClosure(const NFA& nAutomaton, const std::set<int>& initialStates) {
    std::set<int> closure;
    std::queue<int> statesQueue;

    // Adds the elements in the given set to the closure
    for (const auto& state : initialStates) {
        statesQueue.push(state);
        closure.insert(state);
    }

    while (!statesQueue.empty()) {
        int state = statesQueue.front();
        statesQueue.pop();

        // If there is an ɛ-transition the reached states are added to the closure
        auto transitions = nAutomaton.getTransition(state, '-');

        if (!transitions.empty()) {
            statesQueue.push(transitions[0]);
            closure.insert(transitions[0]);

            if (transitions[1] != -1) {
                statesQueue.push(transitions[1]);
                closure.insert(transitions[1]);
            }
        }
    };

    return closure;
}

/// Function that converts from NFA to DFA
DFA subsetConstruction(const std::set<char>& alphabet, const NFA& nAutomaton) {
    std::queue<std::set<int>> unprocessedStates;
    std::set<std::set<int>> computed;

    DFA result;

    unprocessedStates.push(epsilonClosure(nAutomaton, {nAutomaton.getInitialState()}));
    std::set<int> finalStates = nAutomaton.getFinalStates();

    std::map<std::set<int>, int> subsetStateMappings;
    int state = 0;

    subsetStateMappings[unprocessedStates.front()] = state++;
    result.addState();

    while (!unprocessedStates.empty()) {
        auto currentSubset = unprocessedStates.front();
        unprocessedStates.pop();

        for (auto inputChar : alphabet) {
            bool isFinal = false;
            std::set<int> T = std::set<int>();


            for (const int& state : currentSubset) {
                std::vector<int> trans = nAutomaton.getTransition(state, inputChar);

                if (!trans.empty()) {
                    for (int s : trans) {
                        if (s >= 0) {
                            T.insert(s);
                        }
                    }
                }
            }

            T = epsilonClosure(nAutomaton, T);

            if (!T.empty()) {
                for (const auto& s : T) {
                    if (finalStates.find(s) != finalStates.end()) {
                        isFinal = true;
                    }
                }

                if (computed.find(T) == computed.end()) {
                    unprocessedStates.push(T);
                    computed.insert(T);

                    subsetStateMappings[T] = state++;
                    result.addState();
                }

                result.setTransition(subsetStateMappings[currentSubset], inputChar, subsetStateMappings[T]);

                if (isFinal) {
                    result.addFinalState(subsetStateMappings[T]);
                }
            }
        }
    }

    return result;
}

/// Main function used to test subsetConstruction function
int main(int argc, char *argv[]) {
    static std::ifstream infile;
    static std::ofstream outfile;

    NFA nAutomaton;
    DFA dAutomaton;

    bool returnNondeterministic = false;

    if (argc > 1) {
        try {
            for (int i = 1; i < argc; ++i) {
                if (argv[i] == std::string("--input")) {
                    infile.open(argv[++i]);
                } else if (argv[i] == std::string("--output")) {
                    outfile.open(argv[++i]);
                } else if (argv[i] == std::string("-n")) {
                    returnNondeterministic = true;
                }
            }

            if (!outfile.is_open()) {
                outfile.open("output.dot");
            }

            if (!infile.is_open()) {
                throw std::ios_base::failure("Error in opening the input file");
            } else if (!outfile.is_open()) {
                throw std::ios_base::failure("Error in opening the output file");
            }

            nAutomaton.loadState(infile);

            if (returnNondeterministic) {
                nAutomaton.toDOT(outfile);
            } else {
                dAutomaton = subsetConstruction(nAutomaton.getAlphabet(), nAutomaton);
                dAutomaton.toDOT(outfile);
            }

            outfile.close();

            std::cout << "Parse successfull" << std::endl;
        } catch(std::ios_base::failure err) {
            std::cerr << err.what() << std::endl;
        }
    } else {
        std::cerr << "You must provide at least an input file with the non deterministic finite automaton data" << std::endl;
        std::cout << "This utility can create a .dot file given a configuration file for a non-deterministic finite automata" << std::endl;
        std::cout << "It is possible to create a .dot file of the NFA using the -n flag," << std::endl;
        std::cout << "otherwise will be returned the .dot file of the DFA obtained using subset construction algorithm" << std::endl;
        std::cout << "Example: ./subset_construction --input your_input_file [--output your_output_file] [-n]" << std::endl;
    }

    return 0;
}
