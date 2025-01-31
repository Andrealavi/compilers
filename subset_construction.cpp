#include <iostream>
#include <fstream>
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
        DFA(int s) : numStates(s) {
            for (int i = 0; i < numStates; i++) {
                transitions.push_back(std::map<char, int>());
            }
        }

        DFA() {}

        DFA(int s, int initialState) : numStates(s), initialState(initialState) {
            transitions = {};

            for (int i = 0; i < numStates; i++) {
                transitions.push_back(std::map<char, int>());
            }
        }

        // Getters
        int getInitialState() const { return initialState; }

        int getNumStates() const { return numStates; }

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

        int getNumStates() const { return numStates; }

        std::vector<char> getInputChars() const { return input_chars; }

        std::set<int> getFinalStates() const { return finalStates; }

        std::vector<int> getTransition(int state, char input_char) const {
            if (input_chars[state] == input_char) {
                std::vector<int> transitions = {first_state[state], second_state[state]};

                return transitions;
            }

            return {};
        }

        // Setters
        void setTransition(int state, char input_char, std::vector<int> transitions) {
            input_chars[state] = input_char;

            first_state[state] = transitions[0];
            second_state[state] = transitions[1];
        }
};

/// Functions that computes the ɛ-closure of a set of states
std::set<int> epsilonClosure(const NFA& nAutomaton, const std::set<int>& initialStates) {
    std::set<int> closure;
    std::queue<int> statesQueue;

    for (const auto& state : initialStates) {
        statesQueue.push(state);
        closure.insert(state);
    }

    while (!statesQueue.empty()) {
        int state = statesQueue.front();
        statesQueue.pop();

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
int main(int argc, char **argv) {

    std::set<char> alphabet = {'a', 'b', 'c'};

    std::vector<char> ic = {'-', 'a', 'b', '-', '-', 'a', '-', 'c', '-', ' '};
    std::vector<int> fs = {1, 2, 3, 9, 5, 6, 5, 8, 9, -1};
    std::vector<int> ss = {4, -1, -1, -1, 7, -1, 7, -1, -1, -1};
    std::set<int> finalStates = {9};

    NFA prova = NFA(ic, fs, ss, finalStates);

    DFA prova2 = subsetConstruction(alphabet, prova);
    auto transitions = prova2.getTransitions();

    int i = 0;

    for (std::map<char, int> t : transitions) {
        std::cout << i++ << std::endl;
        for (auto const& [k, v] : t) {
            std::cout << k << ": " << v << std::endl;
        }
    }

    for (const int& s : prova2.getFinalStates()) {
        std::cout << s << std::endl;
    }


    return 0;
}
