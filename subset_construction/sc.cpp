/**
 * @file sc.cpp
 * @brief Implementation of NFA to DFA conversion using subset construction algorithm
 * @author Original code modified with documentation
 * @date February 2025
 *
 * This file provides an implementation of the subset construction algorithm
 * for converting a Nondeterministic Finite Automaton (NFA) to a
 * Deterministic Finite Automaton (DFA). It includes classes for both
 * NFA and DFA representations, along with conversion utilities.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <queue>
#include <map>
#include <set>
#include <vector>

/**
 * @class DFA
 * @brief Class representing a Deterministic Finite Automaton
 *
 * This class implements a DFA with states, transitions, and final states.
 * It provides functionality to build and manipulate the automaton, as well
 * as export it to DOT format for visualization.
 */
class DFA {
    private:
        int initialState = 0;                              ///< The initial state of the DFA
        int numStates = 0;                                ///< Total number of states in the DFA
        std::vector<std::map<char, int>> transitions;     ///< Transition function stored as adjacency list
        std::set<int> finalStates;                        ///< Set of final/accepting states

    public:
        /**
         * @brief Default constructor
         */
        DFA() {}

        /**
         * @brief Get the set of final states
         * @return Set of integers representing final states
         */
        std::set<int> getFinalStates() const { return finalStates; }

        /**
         * @brief Get all transitions of the DFA
         * @return Vector of maps representing the transition function
         */
        std::vector<std::map<char, int>> getTransitions() const { return transitions; }

        /**
         * @brief Get the transition for a specific state and input character
         * @param state The current state
         * @param input_char The input character
         * @return Optional containing the next state, or nullopt if invalid
         */
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

        /**
         * @brief Add a new state to the DFA
         */
        void addState() {
            numStates++;
            transitions.push_back(std::map<char,int>());
        }

        /**
         * @brief Add a state to the set of final states
         * @param state The state to be marked as final
         */
        void addFinalState(int state) { finalStates.insert(state); }

        /**
         * @brief Set a transition in the DFA
         * @param state Source state
         * @param input_char Input character
         * @param transition_state Destination state
         */
        void setTransition(int state, char input_char, int transition_state) {
            transitions[state][input_char] = transition_state;
        }

        /**
         * @brief Export the DFA to DOT format
         * @param file Output stream to write the DOT representation
         *
         * Generates a DOT format representation of the DFA that can be
         * used with Graphviz to visualize the automaton.
         */
        void toDOT(std::ostream& file) {
            file << "digraph DFA {" << std::endl;
            file << "rankdir=LR" << std::endl;
            file << "\"\" [shape=none, label=\"\"]" << std::endl;

            int state = 0;
            std::string nodeConfig;

            // Define node shapes based on state properties
            for (int i = 0; i < numStates; i++) {
                if (finalStates.find(i) != finalStates.end()) {
                    file << i << "[shape=doublecircle]" << std::endl;
                } else if (i == initialState) {
                    file << "\"\" -> " << i << std::endl;
                } else {
                    file << i << "[shape=circle]" << std::endl;
                }
            }

            // Add transitions
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

/**
 * @class NFA
 * @brief Class representing a Nondeterministic Finite Automaton
 *
 * This class implements an NFA with states, transitions (including ε-transitions),
 * and final states. It supports loading from a file and conversion to DOT format.
 */
class NFA {
    private:
        int numStates;                    ///< Total number of states in the NFA
        int initialState = 0;             ///< Initial state of the NFA
        std::vector<char> input_chars;    ///< Input characters for each transition
        std::vector<int> first_state;     ///< First possible transition state
        std::vector<int> second_state;    ///< Second possible transition state (for nondeterminism)
        std::set<int> finalStates;        ///< Set of final/accepting states

    public:
        /**
         * @brief Default constructor
         */
        NFA() {}

        /**
         * @brief Constructor with number of states
         * @param numStates Number of states in the NFA
         */
        NFA(int numStates) : numStates(numStates) {}

        /**
         * @brief Constructor with initial state and number of states
         * @param initialState The initial state
         * @param numStates Total number of states
         */
        NFA(int initialState, int numStates) : initialState(initialState), numStates(numStates) {
            input_chars.resize(numStates);
            first_state.resize(numStates);
            second_state.resize(numStates);
        }

        /**
         * @brief Constructor with full NFA definition
         * @param ic Vector of input characters
         * @param fs Vector of first transition states
         * @param ss Vector of second transition states
         * @param finalStates Set of final states
         */
        NFA(std::vector<char> ic, std::vector<int> fs, std::vector<int> ss, std::set<int> finalStates)
            : input_chars(ic), first_state(fs), second_state(ss), finalStates(finalStates) {
                numStates = input_chars.size();
            }

        /**
         * @brief Get the initial state
         * @return The initial state
         */
        int getInitialState() const { return initialState; }

        /**
         * @brief Get the set of final states
         * @return Set of final states
         */
        std::set<int> getFinalStates() const { return finalStates; }

        /**
         * @brief Get all possible transitions for a state and input
         * @param state Current state
         * @param input_char Input character
         * @return Vector of possible next states
         */
        std::vector<int> getTransition(int state, char input_char) const {
            if (input_chars[state] == input_char) {
                std::vector<int> transitions = {first_state[state], second_state[state]};
                return transitions;
            }
            return {};
        }

        /**
         * @brief Get the alphabet of the NFA
         * @return Set of characters in the NFA's alphabet
         */
        std::set<char> getAlphabet() const {
            std::set<char> alphabet;
            for (const char c : input_chars) {
                if (alphabet.find(c) == alphabet.end() && c != '-' && c != ' ') {
                    alphabet.insert(c);
                }
            }
            return alphabet;
        }

        /**
         * @brief Load NFA definition from a file
         * @param file Input file stream
         * @return True if loading was successful
         *
         * File format:
         * - First line: number of states
         * - Second line: initial state
         * - Next lines: input_char,first_state,second_state (one per state)
         * - Final line: list of final states terminated by -1
         */
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

        /**
         * @brief Export the NFA to DOT format
         * @param file Output stream to write the DOT representation
         */
        void toDOT(std::ostream& file) {
            file << "digraph NFA {" << std::endl;
            file << "rankdir=LR" << std::endl;
            file << "\"\" [shape=none, label=\"\"]" << std::endl;

            // Define node shapes
            for (int i = 0; i < numStates; i++) {
                if (finalStates.find(i) != finalStates.end()) {
                    file << i << "[shape=doublecircle]" << std::endl;
                } else if (i == initialState) {
                    file << "\"\" -> " << i << std::endl;
                } else {
                    file << i << "[shape=circle]" << std::endl;
                }
            }

            // Add transitions
            for (int i = 0; i < numStates; i++) {
                std::string label;

                if (input_chars[i] == '-') {
                    label = "ɛ";
                } else {
                    label = input_chars[i];
                }

                if (first_state[i] >= 0)
                    file << i << " -> " << first_state[i] << " [label=\"" << label << "\"]" << std::endl;
                if (second_state[i] >= 0)
                    file << i << " -> " << second_state[i] << " [label=\"" << label << "\"]" << std::endl;
            }

            file << "}" << std::endl;
        }
};

/**
 * @brief Compute the ε-closure of a set of states
 * @param nAutomaton The NFA
 * @param initialStates Set of initial states
 * @return Set of states reachable through ε-transitions
 *
 * This function computes the ε-closure of a set of states in an NFA.
 * The ε-closure includes all states that can be reached from the initial
 * states by following zero or more ε-transitions.
 */
std::set<int> epsilonClosure(const NFA& nAutomaton, const std::set<int>& initialStates) {
    std::set<int> closure;
    std::queue<int> statesQueue;

    // Add initial states to closure
    for (const auto& state : initialStates) {
        statesQueue.push(state);
        closure.insert(state);
    }

    // Process queue until empty
    while (!statesQueue.empty()) {
        int state = statesQueue.front();
        statesQueue.pop();

        // Check for ε-transitions
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

/**
 * @brief Convert NFA to DFA using subset construction algorithm
 * @param alphabet The input alphabet
 * @param nAutomaton The input NFA
 * @return Equivalent DFA
 *
 * This function implements the subset construction algorithm to convert
 * an NFA to an equivalent DFA. The algorithm works by:
 * 1. Computing ε-closures
 * 2. Creating new DFA states for each subset of NFA states
 * 3. Computing transitions between these new states
 */
DFA subsetConstruction(const std::set<char>& alphabet, const NFA& nAutomaton) {
    std::queue<std::set<int>> unprocessedStates;
    std::set<std::set<int>> computed;
    DFA result;

    // Start with ε-closure of initial state
    unprocessedStates.push(epsilonClosure(nAutomaton, {nAutomaton.getInitialState()}));
    std::set<int> finalStates = nAutomaton.getFinalStates();

    // Map to track correspondence between NFA state subsets and DFA states
    std::map<std::set<int>, int> subsetStateMappings;
    int state = 0;

    // Initialize first state
    subsetStateMappings[unprocessedStates.front()] = state++;
    result.addState();

    // Process all state subsets
    while (!unprocessedStates.empty()) {
        auto currentSubset = unprocessedStates.front();
        unprocessedStates.pop();

        // For each input symbol
        for (auto inputChar : alphabet) {
            bool isFinal = false;
            std::set<int> T = std::set<int>();

            // Compute transitions for each state in the subset
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

            // Compute ε-closure of reached states
            T = epsilonClosure(nAutomaton, T);

            if (!T.empty()) {
                // Check if any state in T is final
                for (const auto& s : T) {
                    if (finalStates.find(s) != finalStates.end()) {
                        isFinal = true;
                    }
                }

                // Add new state if not seen before
                if (computed.find(T) == computed.end()) {
                    unprocessedStates.push(T);
                    computed.insert(T);

                    subsetStateMappings[T] = state++;
                    result.addState();
                }

                // Add transition and mark as final if necessary
                result.setTransition(subsetStateMappings[currentSubset], inputChar, subsetStateMappings[T]);

                if (isFinal) {
                    result.addFinalState(subsetStateMappings[T]);
                }
            }
        }
    }

    return result;
}

/**
 * @brief Main function for the NFA to DFA converter
 * @param argc Number of command line arguments
 * @param argv Array of command line arguments
 * @return 0 on success, non-zero on failure
 *
 * Command line arguments:
 * --input <file>   Input file containing NFA definition
 * --output <file>  Output file for DOT representation (default: output.dot)
 * -n              Output NFA instead of converting to DFA
 *
 * Usage example:
 * @code
 * ./subset_construction --input nfa.txt --output automaton.dot [-n]
 * @endcode
 */
int main(int argc, char *argv[]) {
    static std::ifstream infile;
    static std::ofstream outfile;

    NFA nAutomaton;
    DFA dAutomaton;

    bool returnNondeterministic = false;

    if (argc > 1) {
        try {
            // Parse command line arguments
            for (int i = 1; i < argc; ++i) {
                if (argv[i] == std::string("--input")) {
                    infile.open(argv[++i]);
                } else if (argv[i] == std::string("--output")) {
                    outfile.open(argv[++i]);
                } else if (argv[i] == std::string("-n")) {
                    returnNondeterministic = true;
                }
            }

            // Set default output file if not specified
            if (!outfile.is_open()) {
                outfile.open("output.dot");
            }

            // Validate file operations
            if (!infile.is_open()) {
                throw std::ios_base::failure("Error in opening the input file");
            } else if (!outfile.is_open()) {
                throw std::ios_base::failure("Error in opening the output file");
            }

            // Load and process automaton
            nAutomaton.loadState(infile);

            if (returnNondeterministic) {
                nAutomaton.toDOT(outfile);
            } else {
                dAutomaton = subsetConstruction(nAutomaton.getAlphabet(), nAutomaton);
                dAutomaton.toDOT(outfile);
            }

            outfile.close();

            std::cout << "Parse successful" << std::endl;
        } catch(std::ios_base::failure err) {
            std::cerr << err.what() << std::endl;
        }
    } else {
        // Display usage information
        std::cerr << "Error: No input file specified\n" << std::endl;

        std::cout << "NFA/DFA Converter - Subset Construction Algorithm Implementation\n" << std::endl;

        std::cout << "USAGE:\n"
                 << "    subset_construction --input <file> [OPTIONS]\n" << std::endl;

        std::cout << "DESCRIPTION:\n"
                 << "    Converts a Non-deterministic Finite Automaton (NFA) to a Deterministic\n"
                 << "    Finite Automaton (DFA) using the subset construction algorithm.\n"
                 << "    Outputs the result in DOT format for visualization.\n" << std::endl;

        std::cout << "REQUIRED ARGUMENTS:\n"
                 << "    --input <file>    Input file containing NFA definition\n" << std::endl;

        std::cout << "OPTIONS:\n"
                 << "    --output <file>   Output DOT file (default: output.dot)\n"
                 << "    -n                Output the original NFA instead of converting to DFA\n" << std::endl;

        std::cout << "INPUT FILE FORMAT:\n"
                 << "    Line 1: <number_of_states>\n"
                 << "    Line 2: <initial_state>\n"
                 << "    Next lines: <input_char>,<first_state>,<second_state>\n"
                 << "    Last line: <final_states> -1\n" << std::endl;

        std::cout << "EXAMPLES:\n"
                 << "    Basic conversion:\n"
                 << "        ./subset_construction --input nfa.txt\n\n"
                 << "    Specify output file:\n"
                 << "        ./subset_construction --input nfa.txt --output automaton.dot\n\n"
                 << "    Generate NFA visualization:\n"
                 << "        ./subset_construction --input nfa.txt -n\n" << std::endl;
    }

    return 0;
}
