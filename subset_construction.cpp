#include <algorithm>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <queue>
#include <vector>

/// DFA - Class that represents a Deterministic Finite Automaton
class DFA {
    private:
        int initialState = 0;
        std::set<char> alphabet;
        int numStates = 0;
        std::vector<std::map<char, int>> transitions = {};
        std::set<int> finalStates;

    public:
        DFA(std::set<char> a, int s) : alphabet(a), numStates(s) {
            for (int i = 0; i < numStates; i++) {
                transitions.push_back(std::map<char, int>());
            }
        }

        DFA(std::set<char> a) : alphabet(a) {}

        DFA(std::set<char> a, int s, int initialState) : alphabet(a), numStates(s), initialState(initialState) {
            transitions = {};

            for (int i = 0; i < numStates; i++) {
                transitions.push_back(std::map<char, int>());
            }
        }

        void addState() {
            numStates++;

            transitions.push_back(std::map<char,int>());
        }

        int getTransition(int state, char input_char) {
            if (state < 0 && state > numStates) {
                std::cerr << "State index out of range" << std::endl;
                return -1;
            } else if (transitions[state].find(input_char) == transitions[state].end()) {
                std::cerr << "There is no transition for the given state and input char" << std::endl;
                return -1;
            }


            return transitions[state][input_char];
        }

        std::vector<std::map<char, int>> getTransitions() { return transitions; }

        int getInitialState() { return initialState; }

        void addFinalState(int state) { finalStates.insert(state); }

        std::set<int> getFinalStates() { return finalStates; }

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

        int getInitialState() { return initialState; }

        std::vector<char> getInputChars() { return input_chars; }

        std::vector<int> getTransition(int state, char input_char) {
            if (input_chars[state] == input_char) {
                std::vector<int> transitions = {first_state[state], second_state[state]};

                return transitions;
            }

            return {};
        }

        std::set<int> getFinalStates() { return finalStates; }

        void setTransition(int state, char input_char, std::vector<int> transitions) {
            input_chars[state] = input_char;

            first_state[state] = transitions[0];
            second_state[state] = transitions[1];
        }
};

/// Functions that computes the ɛ-closure of a set of states
std::set<int>* epsilonClosure(NFA* nAutomaton, std::set<int>* closureSet) {
    std::set<int>* epsClosureSet = new std::set<int>;
    std::queue<int>* stateQueue = new std::queue<int>;

    for (const int& state : *closureSet) {
        stateQueue->push(state);
        epsClosureSet->insert(state);
    }

    while (!stateQueue->empty()) {
        int state = stateQueue->front();
        stateQueue->pop();

        std::vector<int> transitions = nAutomaton->getTransition(state, '-');

        if (!transitions.empty()) {
            stateQueue->push(transitions[0]);
            epsClosureSet->insert(transitions[0]);

            if (transitions[1] != -1) {
                stateQueue->push(transitions[1]);
                epsClosureSet->insert(transitions[1]);
            }
        }
    };

    return epsClosureSet;
}

/// Support function for subsetConstruction function
bool hasEqualSet(std::queue<std::set<int>*>* queue, std::set<int>* targetSet) {
    std::queue<std::set<int>*> tempQueue = *queue;  // Create a copy

    while (!tempQueue.empty()) {
        if (*tempQueue.front() == *targetSet) {  // Compare set contents
            return true;
        }
        tempQueue.pop();
    }
    return false;
}

/// Support function for the subsetConstruction function
int getIndex(std::map<std::set<int>*, int> subsetsState, std::set<int>* s) {
    for (auto const& [k, v] : subsetsState) {
        if (*k == *s) {
            return v;
        }
    }

    return -1;
}

/// Function that converts from NFA to DFA
DFA subsetConstruction(std::set<char> alphabet, NFA nAutomaton) {
    std::queue<std::set<int>*>* nonVisitedStates = new std::queue<std::set<int>*>;
    std::queue<std::set<int>*>* alreadySeen = new std::queue<std::set<int>*>();

    DFA automaton = DFA(alphabet);

    std::set<int>* initialStateSet = new std::set<int>;
    initialStateSet->insert(nAutomaton.getInitialState());

    nonVisitedStates->push(epsilonClosure(&nAutomaton, initialStateSet));
    std::set<int> finalStates = nAutomaton.getFinalStates();

    std::map<std::set<int>*, int> subsetsState;
    int state = 0;

    subsetsState[initialStateSet] = state++;
    automaton.addState();

    while (!nonVisitedStates->empty()) {
        std::set<int>* statesSubset = nonVisitedStates->front();

        for (char inputChar : alphabet) {
            bool isFinal = false;
            std::set<int>* T = new std::set<int>;

            for (const int& state : *statesSubset) {
                std::vector<int> trans = nAutomaton.getTransition(state, inputChar);

                if (!trans.empty()) {
                    for (int s : trans) {
                        if (s >= 0) {
                            T->insert(s);
                        }
                    }
                }
            }

            T = epsilonClosure(&nAutomaton, T);

            for (const int& s : *T) {
                if (finalStates.find(s) != finalStates.end()) {
                    isFinal = true;
                }
            }

            int k;

            if (!T->empty() && !hasEqualSet(alreadySeen, T)) {
                nonVisitedStates->push(T);
                alreadySeen->push(T);

                k = state;

                subsetsState[T] = state++;
                automaton.addState();
            } else {
                k = getIndex(subsetsState, T);
            }

            automaton.setTransition(subsetsState[statesSubset], inputChar, k);
            if (isFinal) {
                automaton.addFinalState(k);
            }
        }

        nonVisitedStates->pop();
    }

    return automaton;
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
    std::vector<std::map<char, int>> transitions = prova2.getTransitions();

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
