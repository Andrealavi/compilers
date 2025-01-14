#include <iostream>
#include <set>

/*
This program should identify the language L2 = {x \in B* : |x| = 2} = {00, 01, 10, 11}
*/

// Make use of the set data structure
std::set<std::string> L2 = {"00", "01", "10", "11"};

int main(int argc, char **argv) {

    if (argc > 1) {
        // If the given string is within the set the result is accepted
        if (L2.count(argv[1]) == 1) {
            std::cout << "Accept" << std::endl;
        } else {
            std::cout << "Reject" << std::endl;
        }
    } else {
        std::cout << "Missing arguement" << std::endl;

        return 1;
    }

    return 0;
}
